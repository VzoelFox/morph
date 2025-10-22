import interpreter.ast_nodes as ast
from .environment import Environment
from .callable import VzoelCallable, VzoelFunction
from .errors import VzoelRuntimeException, VzoelModuleNotFound, Return
from .token_types import TokenType
from pathlib import Path

class Lihat(VzoelCallable):
    def arity(self) -> int: return 1
    def call(self, interpreter, arguments):
        print(arguments[0])
        return None

class Interpreter:
    def __init__(self):
        self.globals = Environment()
        self.environment = self.globals
        self.globals.define("lihat", Lihat())
    def interpret(self, program: ast.Program):
        try:
            for statement in program.statements:
                self._execute(statement)
        except VzoelRuntimeException as e:
            print(f"Error runtime di baris {e.token.line}: {e.message}")
    def execute_block(self, statements: list[ast.Statement], environment: Environment):
        previous = self.environment
        try:
            self.environment = environment
            for statement in statements:
                self._execute(statement)
        finally:
            self.environment = previous
    def _execute(self, stmt: ast.Statement): stmt.accept(self)
    def _evaluate(self, expr: ast.Expression): return expr.accept(self)

    def visit_AturStatement(self, stmt: ast.AturStatement):
        try:
            value = self._evaluate(stmt.initializer)
            self.environment.define(stmt.name.literal, value)
        except VzoelRuntimeException:
            if stmt.fallback: self._execute(stmt.fallback)
            else: raise
    def visit_AmbilExpression(self, expr: ast.AmbilExpression):
        path_str = self._evaluate(expr.path)
        path = Path(path_str).resolve()
        if not path.exists(): raise VzoelModuleNotFound(expr.keyword, f"Modul tidak ditemukan: {path}")
        with open(path, 'r', encoding='utf-8') as f: source_code = f.read()
        from .lexer import Lexer
        from .parser import Parser
        module_interpreter = Interpreter()
        module_interpreter.interpret(Parser(Lexer(source_code).scan_tokens()).parse())
        return module_interpreter.environment
    def visit_GetExpression(self, expr: ast.GetExpression):
        objek = self._evaluate(expr.objek)
        if isinstance(objek, Environment): return objek.get(expr.name)
        raise VzoelRuntimeException(expr.name, "Hanya environment modul yang punya properti.")
    def visit_ExpressionStatement(self, stmt: ast.ExpressionStatement): self._evaluate(stmt.expression)
    def visit_Literal(self, expr: ast.Literal): return expr.value
    def visit_Variable(self, expr: ast.Variable): return self.environment.get(expr.name)
    def visit_FunctionCall(self, expr: ast.FunctionCall):
        callee = self._evaluate(expr.callee)
        arguments = [self._evaluate(arg) for arg in expr.arguments]
        if not isinstance(callee, VzoelCallable): raise Exception("Hanya fungsi yang bisa dipanggil.")
        if len(arguments) != callee.arity(): raise Exception(f"Diharapkan {callee.arity()} argumen, tapi dapat {len(arguments)}.")
        return callee.call(self, arguments)
    def visit_BlokStatement(self, stmt: ast.BlokStatement):
        self.execute_block(stmt.statements, Environment(enclosing=self.environment))
    def visit_ProsesStatement(self, stmt: ast.ProsesStatement):
        fungsi = VzoelFunction(stmt, self.environment)
        self.environment.define(stmt.name.literal, fungsi)
    def visit_KembaliStatement(self, stmt: ast.KembaliStatement):
        value = self._evaluate(stmt.value) if stmt.value is not None else None
        raise Return(value)
    def visit_BinaryExpression(self, expr: ast.BinaryExpression):
        left = self._evaluate(expr.left)
        right = self._evaluate(expr.right)
        if expr.operator.type == TokenType.PLUS:
            if isinstance(left, (int, float)) and isinstance(right, (int, float)): return left + right
            if isinstance(left, str) and isinstance(right, str): return left + right
            raise VzoelRuntimeException(expr.operator, "Operand harus angka atau string.")
        return None
