# interpreter/interpreter.py
from typing import List, Any
from .environment import Environment
from .callable import VzoelCallable, VzoelFunction, Lihat
from .builtins import Panjang, Tambah, Potong
from .errors import VzoelRuntimeException, VzoelModuleNotFound, Return
from .token_types import TokenType
import interpreter.ast_nodes as ast
from pathlib import Path

class Interpreter:
    def __init__(self):
        self.globals = Environment()
        self.environment = self.globals
        self.globals.define("lihat", Lihat())
        self.globals.define("panjang", Panjang())
        self.globals.define("tambah", Tambah())
        self.globals.define("potong", Potong())

    def interpret(self, program: ast.Program):
        try:
            for stmt in program.statements:
                self._execute(stmt)
        except VzoelRuntimeException as e:
            print(f"Error runtime: {e.message}")

    def execute_block(self, statements: List[ast.Statement], environment: Environment):
        previous = self.environment
        try:
            self.environment = environment
            for stmt in statements:
                self._execute(stmt)
        finally:
            self.environment = previous

    def _execute(self, stmt: ast.Statement):
        stmt.accept(self)

    def _evaluate(self, expr: ast.Expression):
        return expr.accept(self)

    def visit_ExpressionStatement(self, stmt: ast.ExpressionStatement):
        self._evaluate(stmt.expression)

    def visit_AturStatement(self, stmt: ast.AturStatement):
        try:
            value = self._evaluate(stmt.initializer)
            self.environment.define(stmt.name.literal, value)
        except (VzoelRuntimeException, VzoelModuleNotFound):
            if stmt.fallback:
                self._execute(stmt.fallback)
            else:
                raise

    def visit_ProsesStatement(self, stmt: ast.ProsesStatement):
        self.environment.define(stmt.name.literal, VzoelFunction(stmt, self.environment))

    def visit_BlokStatement(self, stmt: ast.BlokStatement):
        self.execute_block(stmt.statements, Environment(enclosing=self.environment))

    def visit_KembaliStatement(self, stmt: ast.KembaliStatement):
        value = self._evaluate(stmt.value) if stmt.value else None
        raise Return(value)

    def visit_UlangiStatement(self, stmt: ast.UlangiStatement):
        count = self._evaluate(stmt.count)
        if not isinstance(count, (int, float)):
            raise VzoelRuntimeException(None, "Jumlah perulangan harus berupa angka.")

        for _ in range(int(count)):
            self._execute(stmt.body)

    def visit_Literal(self, expr: ast.Literal):
        return expr.value

    def visit_Variable(self, expr: ast.Variable):
        return self.environment.get(expr.name)

    def visit_FunctionCall(self, expr: ast.FunctionCall):
        callee = self._evaluate(expr.callee)
        args = []
        for arg in expr.arguments:
            args.append(self._evaluate(arg))

        if not isinstance(callee, VzoelCallable):
            raise VzoelRuntimeException(None, "Hanya bisa memanggil proses.")

        if len(args) != callee.arity():
            raise VzoelRuntimeException(None, f"Diharapkan {callee.arity()} argumen tapi dapat {len(args)}.")

        return callee.call(self, args)

    def visit_BinaryExpression(self, expr: ast.BinaryExpression):
        left = self._evaluate(expr.left)
        right = self._evaluate(expr.right)
        op = expr.operator.type
        if op == TokenType.PLUS: return left + right
        if op == TokenType.MINUS: return left - right
        if op == TokenType.BINTANG: return left * right
        if op == TokenType.GARIS_MIRING: return left / right
        if op == TokenType.LEBIH_DARI: return left > right
        if op == TokenType.KURANG_DARI: return left < right
        if op == TokenType.SAMA_DENGAN_SAMA_DENGAN: return left == right
        if op == TokenType.TIDAK_SAMA_DENGAN: return left != right
        return None

    def visit_UnaryExpression(self, expr: ast.UnaryExpression):
        right = self._evaluate(expr.right)
        if expr.operator.type == TokenType.MINUS: return -right
        return None

    def visit_Grouping(self, expr: ast.Grouping):
        return self._evaluate(expr.expression)

    def visit_ListLiteral(self, expr: ast.ListLiteral):
        elements = []
        for element_expr in expr.elements:
            elements.append(self._evaluate(element_expr))
        return elements

    def visit_AmbilExpression(self, expr: ast.AmbilExpression):
        path_str = self._evaluate(expr.path)
        path = Path(path_str).resolve()

        if not path.exists() or not path.is_file():
            raise VzoelModuleNotFound(expr.keyword, f"Modul tidak ditemukan di: {path}")

        with open(path, 'r', encoding='utf-8') as f:
            source_code = f.read()

        from .lexer import Lexer
        from .parser import Parser
        module_lexer = Lexer(source_code)
        module_tokens = module_lexer.scan_tokens()
        module_parser = Parser(module_tokens)
        module_ast = module_parser.parse()

        module_interpreter = Interpreter()
        module_interpreter.interpret(module_ast)

        return module_interpreter.environment

    def visit_GetExpression(self, expr: ast.GetExpression):
        objek = self._evaluate(expr.objek)
        if isinstance(objek, Environment):
            return objek.get(expr.name)
        raise VzoelRuntimeException(expr.name, "Hanya environment modul yang memiliki properti.")

    def visit_MapLiteral(self, expr: ast.MapLiteral):
        peta = {}
        for i in range(len(expr.keys)):
            key = self._evaluate(expr.keys[i])
            value = self._evaluate(expr.values[i])
            peta[key] = value
        return peta

    def visit_SubscriptExpression(self, expr: ast.SubscriptExpression):
        objek = self._evaluate(expr.objek)
        indeks = self._evaluate(expr.indeks)

        if isinstance(objek, list):
            if not isinstance(indeks, (int, float)):
                raise VzoelRuntimeException(None, "Indeks daftar harus berupa angka.")
            return objek[int(indeks)]

        if isinstance(objek, dict):
            return objek.get(indeks)

        raise VzoelRuntimeException(None, "Hanya bisa mengakses elemen dari daftar atau peta.")
