import interpreter.ast_nodes as ast
from .environment import Environment
from .callable import VzoelCallable
from .errors import VzoelRuntimeException, VzoelModuleNotFound
import time # Untuk fungsi bawaan nanti
from pathlib import Path

# --- Fungsi Bawaan ---
class Lihat(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments):
        print(arguments[0])
        return None

# --- Interpreter ---
class Interpreter:
    def __init__(self):
        self.globals = Environment()
        self.environment = self.globals

        # Daftarkan fungsi bawaan
        self.globals.define("lihat", Lihat())

    def interpret(self, program: ast.Program):
        try:
            for statement in program.statements:
                self._execute(statement)
        except Exception as e:
            # Error handling sederhana
            print(e)

    def _execute(self, stmt: ast.Statement):
        stmt.accept(self)

    def _evaluate(self, expr: ast.Expression):
        return expr.accept(self)

    # --- Visitor Methods ---

    def visit_AturStatement(self, stmt: ast.AturStatement):
        try:
            value = self._evaluate(stmt.initializer)
            self.environment.define(stmt.name.literal, value)
        except VzoelRuntimeException as e:
            if stmt.fallback:
                self._execute(stmt.fallback)
            else:
                # Jika tidak ada fallback, lempar error
                raise e

    def visit_AmbilExpression(self, expr: ast.AmbilExpression):
        path_str = self._evaluate(expr.path)
        path = Path(path_str).resolve()

        if not path.exists() or not path.is_file():
            raise VzoelModuleNotFound(expr.keyword, f"Modul tidak ditemukan di: {path}")

        with open(path, 'r', encoding='utf-8') as f:
            source_code = f.read()

        # Buat interpreter terpisah untuk modul
        from .lexer import Lexer
        from .parser import Parser
        module_lexer = Lexer(source_code)
        module_tokens = module_lexer.scan_tokens()
        module_parser = Parser(module_tokens)
        module_ast = module_parser.parse()

        module_interpreter = Interpreter()
        module_interpreter.interpret(module_ast)

        return module_interpreter.environment # Kembalikan environment modul

    def visit_GetExpression(self, expr: ast.GetExpression):
        objek = self._evaluate(expr.objek)
        if isinstance(objek, Environment):
            return objek.get(expr.name)

        raise VzoelRuntimeException(expr.name, "Hanya environment modul yang memiliki properti.")

    def visit_ExpressionStatement(self, stmt: ast.ExpressionStatement):
        self._evaluate(stmt.expression)

    def visit_Literal(self, expr: ast.Literal):
        return expr.value

    def visit_Variable(self, expr: ast.Variable):
        return self.environment.get(expr.name)

    def visit_FunctionCall(self, expr: ast.FunctionCall):
        callee = self._evaluate(expr.callee)

        arguments = []
        for argument in expr.arguments:
            arguments.append(self._evaluate(argument))

        if not isinstance(callee, VzoelCallable):
            raise Exception("Hanya fungsi yang bisa dipanggil.")

        # Nanti kita bisa cek arity (jumlah argumen) di sini

        return callee.call(self, arguments)
