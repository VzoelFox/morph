import interpreter.ast_nodes as ast
from .environment import Environment
from .callable import VzoelCallable, VzoelFunction
from .errors import VzoelRuntimeException, Return
from .token_types import TokenType
from typing import List, Any

class Lihat(VzoelCallable):
    def arity(self) -> int: return 1
    def call(self, interpreter, arguments: List[Any]): print(arguments[0]); return None

class Interpreter:
    def __init__(self):
        self.globals = Environment()
        self.environment = self.globals
        self.globals.define("lihat", Lihat())

    def interpret(self, statements: List[ast.Statement]):
        try:
            for stmt in statements: self._execute(stmt)
        except VzoelRuntimeException as e: print(f"Error: {e.message}")

    def execute_block(self, statements: List[ast.Statement], environment: Environment):
        previous = self.environment
        try:
            self.environment = environment
            for stmt in statements: self._execute(stmt)
        finally: self.environment = previous

    def _execute(self, stmt): stmt.accept(self)
    def _evaluate(self, expr): return expr.accept(self)

    def visit_ExpressionStatement(self, stmt): self._evaluate(stmt.expression)
    def visit_AturStatement(self, stmt):
        value = self._evaluate(stmt.initializer)
        self.environment.define(stmt.name.literal, value)
    def visit_ProsesStatement(self, stmt):
        self.environment.define(stmt.name.literal, VzoelFunction(stmt, self.environment))
    def visit_BlokStatement(self, stmt):
        self.execute_block(stmt.statements, Environment(enclosing=self.environment))
    def visit_KembaliStatement(self, stmt):
        value = self._evaluate(stmt.value) if stmt.value else None
        raise Return(value)
    def visit_JikaStatement(self, stmt):
        if self._evaluate(stmt.condition):
            self._execute(stmt.then_branch)

    def visit_Literal(self, expr): return expr.value
    def visit_Variable(self, expr): return self.environment.get(expr.name)
    def visit_FunctionCall(self, expr):
        callee = self._evaluate(expr.callee)
        args = [self._evaluate(arg) for arg in expr.arguments]
        return callee.call(self, args)
    def visit_BinaryExpression(self, expr):
        left = self._evaluate(expr.left); right = self._evaluate(expr.right)
        op = expr.operator.type
        if op == TokenType.PLUS: return left + right
        if op == TokenType.MINUS: return left - right
        if op == TokenType.BINTANG: return left * right
        if op == TokenType.GARIS_MIRING: return left / right
        if op == TokenType.LEBIH_DARI: return left > right
        if op == TokenType.KURANG_DARI: return left < right
        if op == TokenType.SAMA_DENGAN_SAMA_DENGAN: return left == right
        if op == TokenType.TIDAK_SAMA_DENGAN: return left != right
    def visit_UnaryExpression(self, expr):
        right = self._evaluate(expr.right)
        if expr.operator.type == TokenType.MINUS: return -right
        if expr.operator.type == TokenType.TIDAK_LOGIS: return not right
