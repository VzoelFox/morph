# interpreter/interpreter.py
from typing import List, Any
from .environment import Environment
from .callable import VzoelCallable, VzoelFunction, Lihat
from .builtins import Panjang, Tambah, Potong, KeKecil, KeBesar
from .errors import VzoelRuntimeException, VzoelModuleNotFound, Return
from .token_types import TokenType
from .token import Token
import interpreter.ast_nodes as ast
from pathlib import Path

class Interpreter(ast.Visitor):
    def __init__(self):
        self.globals = Environment()
        self.environment = self.globals
        # ... (definisi built-in function tetap sama) ...
        self.globals.define("lihat", Lihat())
        self.globals.define("panjang", Panjang())
        self.globals.define("tambah", Tambah())
        self.globals.define("potong", Potong())
        self.globals.define("akar", Akar())
        self.globals.define("pangkat", Pangkat())

    def interpret(self, program: ast.Program):
        try:
            for stmt in program.statements:
                self._execute(stmt)
        except VzoelRuntimeException as e:
            if e.token:
                print(f"[Baris {e.token.line}] Error runtime: {e.message}")
            else:
                print(f"Error runtime: {e.message}")
        self.globals.define("ke_kecil", KeKecil())
        self.globals.define("ke_besar", KeBesar())

    def interpret(self, program: ast.Program):
        # try...except block removed to allow exceptions to propagate up
        for stmt in program.statements:
            self._execute(stmt)

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

    # --- Implementasi Visitor untuk Statements ---
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
        raise Return(value, stmt.keyword)

    def visit_UlangiStatement(self, stmt: ast.UlangiStatement):
        count_expr = stmt.count
        count = self._evaluate(count_expr)
        if not isinstance(count, (int, float)):
            # Kita butuh token untuk error, kita bisa ambil dari ekspresi count
            # Ini asumsi, cara lebih baik adalah menyimpan token di node UlangiStatement
            raise VzoelRuntimeException(count_expr.operator if hasattr(count_expr, 'operator') else Token(TokenType.NUMBER, str(count), 0, 0), "Jumlah perulangan harus berupa angka.")

        for _ in range(int(count)):
            self._execute(stmt.body)

    # --- Implementasi Visitor untuk Expressions ---
    def visit_Literal(self, expr: ast.Literal):
        return expr.value

    def visit_Variable(self, expr: ast.Variable):
        return self.environment.get(expr.name)

    def visit_FunctionCall(self, expr: ast.FunctionCall):
        callee = self._evaluate(expr.callee)
        args = [self._evaluate(arg) for arg in expr.arguments]

        token = None
        if hasattr(expr.callee, 'name'):
            token = expr.callee.name

        if not isinstance(callee, VzoelCallable):
            raise VzoelRuntimeException(expr.callee.name if isinstance(expr.callee, ast.Variable) else Token(TokenType.IDENTIFIER, "ekspresi", 0), "Hanya bisa memanggil proses atau fungsi bawaan.")

        if len(args) != callee.arity():
            raise VzoelRuntimeException(expr.callee.name, f"Diharapkan {callee.arity()} argumen tapi dapat {len(args)}.")

        return callee.call(self, args)

    def visit_BinaryExpression(self, expr: ast.BinaryExpression):
        left = self._evaluate(expr.left)
        right = self._evaluate(expr.right)
        op_token = expr.operator

        if op_token.type == TokenType.PLUS:
            if isinstance(left, (int, float)) and isinstance(right, (int, float)): return left + right
            if isinstance(left, str) and isinstance(right, str): return left + right
            raise VzoelRuntimeException(op_token, "Operan harus dua angka atau dua string.")

        if op_token.type == TokenType.MINUS:
            self._check_number_operands(op_token, left, right)
            return left - right
        if op_token.type == TokenType.BINTANG:
            self._check_number_operands(op_token, left, right)
            return left * right
        if op_token.type == TokenType.GARIS_MIRING:
            self._check_number_operands(op_token, left, right)
            if right == 0:
                raise VzoelRuntimeException(op_token, "Tidak bisa membagi dengan nol.")
            return left / right

        # Operasi perbandingan
        if op_token.type == TokenType.LEBIH_DARI:
            self._check_number_operands(op_token, left, right)
            return left > right
        if op_token.type == TokenType.KURANG_DARI:
            self._check_number_operands(op_token, left, right)
            return left < right

        if op_token.type == TokenType.SAMA_DENGAN_SAMA_DENGAN: return left == right
        if op_token.type == TokenType.TIDAK_SAMA_DENGAN: return left != right

        return None # Seharusnya tidak pernah terjadi

    def visit_UnaryExpression(self, expr: ast.UnaryExpression):
        right = self._evaluate(expr.right)
        if expr.operator.type == TokenType.MINUS:
            self._check_number_operand(expr.operator, right)
            return -right
        return None

    def visit_Grouping(self, expr: ast.Grouping):
        return self._evaluate(expr.expression)

    def visit_ListLiteral(self, expr: ast.ListLiteral):
        return [self._evaluate(e) for e in expr.elements]

    def visit_AmbilExpression(self, expr: ast.AmbilExpression):
        path_str = expr.path.value # path is a Literal, no need to evaluate
        path = Path(path_str).resolve()

        if not path.exists() or not path.is_file():
            raise VzoelModuleNotFound(expr.keyword, f"Modul tidak ditemukan di: {path}")

        with open(path, 'r', encoding='utf-8') as f:
            source_code = f.read()

        from .lexer import Lexer
        from .parser import Parser
        module_lexer = Lexer(source_code)
        module_tokens = module_lexer.scan_tokens()
        # Periksa error lexer di sini jika perlu
        module_parser = Parser(module_tokens)
        module_ast = module_parser.parse()
        # Periksa error parser di sini jika perlu

        module_interpreter = Interpreter()
        module_interpreter.interpret(module_ast)

        return module_interpreter.globals # Gunakan globals untuk environment modul

    def visit_GetExpression(self, expr: ast.GetExpression):
        objek = self._evaluate(expr.objek)
        if isinstance(objek, Environment):
            return objek.get(expr.name)
        raise VzoelRuntimeException(expr.name, "Hanya environment modul yang memiliki properti.")

    def visit_MapLiteral(self, expr: ast.MapLiteral):
        return {self._evaluate(k): self._evaluate(v) for k, v in zip(expr.keys, expr.values)}

    def visit_SubscriptExpression(self, expr: ast.SubscriptExpression):
        objek = self._evaluate(expr.objek)
        indeks = self._evaluate(expr.indeks)

        # Kita butuh token untuk error, bisa diambil dari token kurung siku
        # Untuk sekarang, kita buat token dummy
        error_token = Token(TokenType.KURUNG_SIKU_TUTUP, ']', expr.objek.name.line if hasattr(expr.objek, 'name') else 0)

        if isinstance(objek, list):
            if not isinstance(indeks, (int, float)):
                raise VzoelRuntimeException(error_token, "Indeks daftar harus berupa angka.")
            idx = int(indeks)
            if not (0 <= idx < len(objek)):
                raise VzoelRuntimeException(error_token, f"Indeks {idx} di luar batas untuk daftar dengan panjang {len(objek)}.")
            return objek[idx]

        if isinstance(objek, dict):
            # Untuk peta, kita bisa kembalikan null jika kunci tidak ada, atau error
            if indeks not in objek:
                 raise VzoelRuntimeException(error_token, f"Kunci '{indeks}' tidak ditemukan di peta.")
            return objek.get(indeks)

        raise VzoelRuntimeException(error_token, "Hanya bisa mengakses elemen dari daftar atau peta.")

    # --- Fungsi Helper untuk Validasi Tipe ---
    def _check_number_operand(self, operator: Token, operand: Any):
        if isinstance(operand, (int, float)): return
        raise VzoelRuntimeException(operator, "Operan harus berupa angka.")

    def _check_number_operands(self, operator: Token, left: Any, right: Any):
        if isinstance(left, (int, float)) and isinstance(right, (int, float)): return
        raise VzoelRuntimeException(operator, "Kedua operan harus berupa angka.")
