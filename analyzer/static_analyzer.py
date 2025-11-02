# analyzer/static_analyzer.py
from typing import Dict, Any, Optional
import sys
import interpreter.ast_nodes as ast
from interpreter.token import Token
from interpreter.token_types import TokenType
from interpreter.errors import VzoelRuntimeException

# Enum sederhana untuk merepresentasikan tipe data dasar
class StaticType:
    ANGKA = "angka"
    STRING = "string"
    BOOLEAN = "boolean"
    DAFTAR = "daftar"
    PETA = "peta"
    PROSES = "proses"
    MODUL = "modul"
    NIHIL = "nihil"
    APAPUN = "apapun" # Untuk kasus dimana tipe belum bisa ditentukan

class TypeEnvironment:
    def __init__(self, enclosing: Optional['TypeEnvironment'] = None):
        self.types: Dict[str, Any] = {}
        self.enclosing = enclosing

    def define(self, name: str, type: Any):
        self.types[name] = type

    def get(self, name_token: Token) -> Any:
        name = name_token.literal
        if name in self.types:
            return self.types[name]
        if self.enclosing is not None:
            return self.enclosing.get(name_token)
        raise VzoelRuntimeException(name_token, f"Variabel '{name}' tidak terdefinisi.")

class StaticAnalyzer:
    def __init__(self):
        self.globals = TypeEnvironment()
        self.environment = self.globals

    def analyze(self, program: ast.Program):
        try:
            program.accept(self)
        except VzoelRuntimeException as e:
            print(f"Error Analisis Statis (L{e.token.line}): {e.message}", file=sys.stderr)
            return False
        return True

    def _execute(self, stmt: ast.Statement):
        stmt.accept(self)

    def _evaluate(self, expr: ast.Expression):
        return expr.accept(self)

    # --- Metode Visitor untuk Pernyataan (Statements) ---

    def visit_Program(self, program: ast.Program):
        for stmt in program.statements:
            self._execute(stmt)

    def visit_ExpressionStatement(self, stmt: ast.ExpressionStatement):
        self._evaluate(stmt.expression)

    def visit_AturStatement(self, stmt: ast.AturStatement):
        inferred_type = self._evaluate(stmt.initializer)
        self.environment.define(stmt.name.literal, inferred_type)
        # Fallback analysis could be complex, skipping for now.

    # --- Metode Visitor untuk Ekspresi (Expressions) ---

    def visit_Variable(self, expr: ast.Variable):
        return self.environment.get(expr.name)

    def visit_Literal(self, expr: ast.Literal):
        if isinstance(expr.value, (int, float)):
            return StaticType.ANGKA
        if isinstance(expr.value, str):
            return StaticType.STRING
        if isinstance(expr.value, bool):
            return StaticType.BOOLEAN
        if expr.value is None:
            return StaticType.NIHIL
        return StaticType.APAPUN

    def visit_BinaryExpression(self, expr: ast.BinaryExpression):
        left_type = self._evaluate(expr.left)
        right_type = self._evaluate(expr.right)
        op = expr.operator.type

        if op == TokenType.PLUS:
            if left_type == StaticType.ANGKA and right_type == StaticType.ANGKA:
                return StaticType.ANGKA
            if left_type == StaticType.STRING and right_type == StaticType.STRING:
                return StaticType.STRING
            raise VzoelRuntimeException(
                expr.operator,
                f"Operasi '+' tidak didukung antara tipe '{left_type}' dan '{right_type}'."
            )

        if op in [TokenType.MINUS, TokenType.BINTANG, TokenType.GARIS_MIRING,
                   TokenType.LEBIH_DARI, TokenType.KURANG_DARI]:
            if left_type != StaticType.ANGKA or right_type != StaticType.ANGKA:
                raise VzoelRuntimeException(
                    expr.operator,
                    f"Operasi '{expr.operator.literal}' memerlukan dua operan tipe '{StaticType.ANGKA}'."
                )
            if op in [TokenType.LEBIH_DARI, TokenType.KURANG_DARI]:
                return StaticType.BOOLEAN
            return StaticType.ANGKA

        if op in [TokenType.SAMA_DENGAN_SAMA_DENGAN, TokenType.TIDAK_SAMA_DENGAN]:
            return StaticType.BOOLEAN

        return StaticType.APAPUN

    def visit_Grouping(self, expr: ast.Grouping):
        return self._evaluate(expr.expression)

    # Placeholder untuk metode visit lain yang belum diimplementasikan
    def visit_BlokStatement(self, stmt: ast.BlokStatement):
        # Menganalisis blok dalam lingkup baru
        previous_env = self.environment
        self.environment = TypeEnvironment(enclosing=previous_env)
        for statement in stmt.statements:
            self._execute(statement)
        self.environment = previous_env

    def visit_ProsesStatement(self, stmt: ast.ProsesStatement):
        # Mendefinisikan proses di lingkungan saat ini
        self.environment.define(stmt.name.literal, StaticType.PROSES)
        # Analisis isi proses di lingkup baru
        previous_env = self.environment
        self.environment = TypeEnvironment(enclosing=previous_env)
        for param in stmt.params:
            self.environment.define(param.literal, StaticType.APAPUN) # Tipe parameter belum didukung
        self._execute(stmt.body)
        self.environment = previous_env

    def visit_KembaliStatement(self, stmt: ast.KembaliStatement):
        if stmt.value:
            self._evaluate(stmt.value)

    def visit_UlangiStatement(self, stmt: ast.UlangiStatement):
        count_type = self._evaluate(stmt.count)
        if count_type != StaticType.ANGKA:
            raise VzoelRuntimeException(
                # Upaya untuk mendapatkan token yang relevan dari ekspresi
                stmt.count.name if hasattr(stmt.count, 'name') else Token(TokenType.NUMBER, str(stmt.count), 0),
                "Jumlah perulangan harus berupa angka."
            )
        self._execute(stmt.body)
    def visit_FunctionCall(self, expr: ast.FunctionCall): return StaticType.APAPUN
    def visit_AmbilExpression(self, expr: ast.AmbilExpression): return StaticType.MODUL
    def visit_GetExpression(self, expr: ast.GetExpression): return StaticType.APAPUN
    def visit_SubscriptExpression(self, expr: ast.SubscriptExpression): return StaticType.APAPUN
    def visit_UnaryExpression(self, expr: ast.UnaryExpression):
        if self._evaluate(expr.right) == StaticType.ANGKA:
            return StaticType.ANGKA
        raise VzoelRuntimeException(expr.operator, "Operasi unary '-' hanya untuk angka.")
    def visit_ListLiteral(self, expr: ast.ListLiteral): return StaticType.DAFTAR
    def visit_MapLiteral(self, expr: ast.MapLiteral): return StaticType.PETA
