# interpreter/interpreter.py
from typing import List, Any
from .environment import Environment
from .callable import VzoelCallable, VzoelFunction, Lihat
from .builtins import UbahKeString, Panjang, Tambah
from .errors import VzoelRuntimeException, VzoelModuleNotFound, Return
from .token_types import TokenType
from .scheduler import Scheduler, Task
import interpreter.ast_nodes as ast
from pathlib import Path

class Interpreter:
    def __init__(self):
        self.globals = Environment()
        self.environment = self.globals
        self.globals.define("lihat", Lihat())
        self.globals.define("ubahKeString", UbahKeString())
        self.globals.define("panjang", Panjang())
        self.globals.define("tambah", Tambah())
        self.scheduler = Scheduler()

    def interpret(self, program: ast.Program):
        try:
            for stmt in program.statements:
                for _ in self._execute(stmt): pass
        except VzoelRuntimeException as e:
            print(f"Error runtime: {e.message}")

    def execute_block(self, statements: List[ast.Statement], environment: Environment):
        previous = self.environment
        try:
            self.environment = environment
            for stmt in statements:
                yield from self._execute(stmt)
        finally:
            self.environment = previous

    def _execute(self, stmt: ast.Statement):
        # Memastikan accept selalu bisa di-iterasi
        result = stmt.accept(self)
        if result is not None:
            yield from result

    def _evaluate(self, expr: ast.Expression):
        value = expr.accept(self)
        if hasattr(value, '__iter__') and not isinstance(value, (str, list, dict)):
            return (yield from value)
        return value

    def visit_ExpressionStatement(self, stmt: ast.ExpressionStatement):
        yield from self._evaluate(stmt.expression)

    def visit_AturStatement(self, stmt: ast.AturStatement):
        try:
            value = yield from self._evaluate(stmt.initializer)
            self.environment.define(stmt.name.literal, value)
        except (VzoelRuntimeException, VzoelModuleNotFound):
            if stmt.fallback:
                yield from self._execute(stmt.fallback)
            else:
                raise
        yield # Generator kosong

    def visit_ProsesStatement(self, stmt: ast.ProsesStatement):
        self.environment.define(stmt.name.literal, VzoelFunction(stmt, self.environment))
        yield # Generator kosong

    def visit_BlokStatement(self, stmt: ast.BlokStatement):
        yield from self.execute_block(stmt.statements, Environment(enclosing=self.environment))

    def visit_KembaliStatement(self, stmt: ast.KembaliStatement):
        value = (yield from self._evaluate(stmt.value)) if stmt.value else None
        raise Return(value)

    def visit_Literal(self, expr: ast.Literal):
        return expr.value

    def visit_Variable(self, expr: ast.Variable):
        return self.environment.get(expr.name)

    def visit_FunctionCall(self, expr: ast.FunctionCall):
        callee = yield from self._evaluate(expr.callee)
        args = []
        for arg in expr.arguments:
            args.append((yield from self._evaluate(arg)))

        if not isinstance(callee, VzoelCallable):
            raise VzoelRuntimeException(None, "Hanya bisa memanggil proses.")

        result = callee.call(self, args)
        if hasattr(result, '__iter__') and not isinstance(result, (str, list, dict)):
             return (yield from result)
        return result

    def visit_BinaryExpression(self, expr: ast.BinaryExpression):
        left = yield from self._evaluate(expr.left)
        right = yield from self._evaluate(expr.right)
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
        right = yield from self._evaluate(expr.right)
        if expr.operator.type == TokenType.MINUS: return -right
        if expr.operator.type == TokenType.TIDAK_LOGIS: return not right
        return None

    def visit_ManagementStatement(self, stmt: ast.ManagementStatement):
        yield from self.execute_block(stmt.body.statements, self.environment)
        self.scheduler.run_until_complete()

    def visit_JalankanExpression(self, expr: ast.JalankanExpression):
        callee = yield from self._evaluate(expr.call.callee)
        args = []
        for arg in expr.call.arguments:
            args.append((yield from self._evaluate(arg)))

        if not isinstance(callee, VzoelFunction):
            raise VzoelRuntimeException(None, "Hanya proses buatan pengguna yang bisa dijalankan.")

        task = Task(callee, args, self)
        self.scheduler.add_task(task)
        return task

    def visit_TungguExpression(self, expr: ast.TungguExpression):
        duration = yield from self._evaluate(expr.duration)
        yield duration

    def visit_Grouping(self, expr: ast.Grouping):
        return (yield from self._evaluate(expr.expression))

    def visit_ListLiteral(self, expr: ast.ListLiteral):
        elements = []
        for element_expr in expr.elements:
            elements.append((yield from self._evaluate(element_expr)))
        return elements

    def visit_AmbilExpression(self, expr: ast.AmbilExpression):
        path_str = yield from self._evaluate(expr.path)
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
        objek = yield from self._evaluate(expr.objek)
        if isinstance(objek, Environment):
            return objek.get(expr.name)
        raise VzoelRuntimeException(expr.name, "Hanya environment modul yang memiliki properti.")
