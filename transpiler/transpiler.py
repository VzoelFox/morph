# transpiler/transpiler.py
import interpreter.ast_nodes as ast
from interpreter.token_types import TokenType

class Transpiler:
    def __init__(self):
        self._indentation_level = 0

    def transpile(self, program: ast.Program) -> str:
        """
        Titik masuk utama untuk proses transpilasi.
        Menerima sebuah Program AST dan mengembalikan string kode Python.
        """
        return self._visit(program)

    def _visit(self, node: ast.ASTNode):
        """Helper untuk memanggil metode visit yang sesuai."""
        method_name = f'visit_{type(node).__name__}'
        visitor = getattr(self, method_name, self._generic_visit)
        return visitor(node)

    def _generic_visit(self, node: ast.ASTNode):
        # Fallback untuk node yang belum diimplementasikan secara eksplisit
        return self._indent() + f"# Node '{type(node).__name__}' belum didukung oleh transpiler."

    def _indent(self):
        return "    " * self._indentation_level

    # --- Metode Visitor untuk Pernyataan (Statements) ---

    def visit_Program(self, program: ast.Program):
        lines = [self._visit(stmt) for stmt in program.statements]
        return "\n".join(lines)

    def visit_ExpressionStatement(self, stmt: ast.ExpressionStatement):
        return self._indent() + self._visit(stmt.expression)

    def visit_AturStatement(self, stmt: ast.AturStatement):
        var_name = stmt.name.literal
        value = self._visit(stmt.initializer)
        return self._indent() + f"{var_name} = {value}"

    def visit_BlokStatement(self, stmt: ast.BlokStatement):
        # Blok tidak menghasilkan indentasi sendiri, hanya kontennya
        lines = [self._visit(s) for s in stmt.statements]
        return "\n".join(lines)

    def visit_UlangiStatement(self, stmt: ast.UlangiStatement):
        count = self._visit(stmt.count)
        header = self._indent() + f"for _ in range(int({count})):"

        self._indentation_level += 1
        body = self._visit(stmt.body)
        self._indentation_level -= 1

        return f"{header}\n{body}"


    # --- Metode Visitor untuk Ekspresi (Expressions) ---

    def visit_Variable(self, expr: ast.Variable):
        return expr.name.literal

    def visit_Literal(self, expr: ast.Literal):
        if expr.value == 'benar': return "True"
        if expr.value == 'salah': return "False"
        if isinstance(expr.value, str): return repr(expr.value)
        if isinstance(expr.value, bool): return str(expr.value)
        if expr.value is None: return "None"
        return str(expr.value)

    def visit_ListLiteral(self, expr: ast.ListLiteral):
        elements = [self._visit(elem) for elem in expr.elements]
        return f"[{', '.join(elements)}]"

    def visit_MapLiteral(self, expr: ast.MapLiteral):
        pairs = [f"{self._visit(k)}: {self._visit(v)}" for k, v in zip(expr.keys, expr.values)]
        return f"{{{', '.join(pairs)}}}"

    def visit_SubscriptExpression(self, expr: ast.SubscriptExpression):
        objek = self._visit(expr.objek)
        indeks = self._visit(expr.indeks)
        return f"{objek}[{indeks}]"

    def visit_BinaryExpression(self, expr: ast.BinaryExpression):
        left = self._visit(expr.left)
        op_map = {TokenType.SAMA_DENGAN_SAMA_DENGAN: "==", TokenType.TIDAK_SAMA_DENGAN: "!="}
        op = op_map.get(expr.operator.type, expr.operator.literal)
        right = self._visit(expr.right)
        return f"({left} {op} {right})"

    def visit_Grouping(self, expr: ast.Grouping):
        return f"({self._visit(expr.expression)})"

    def visit_FunctionCall(self, expr: ast.FunctionCall):
        if not isinstance(expr.callee, ast.Variable):
            return "# Pemanggilan fungsi dinamis belum didukung."

        func_name = expr.callee.name.literal
        args = [self._visit(arg) for arg in expr.arguments]

        if func_name == "lihat":
            if len(args) == 1 and isinstance(expr.arguments[0], ast.BinaryExpression) and expr.arguments[0].operator.type == TokenType.PLUS:
                def build_f_string_parts(sub_expr):
                    if isinstance(sub_expr, ast.BinaryExpression) and sub_expr.operator.type == TokenType.PLUS:
                        return f"{build_f_string_parts(sub_expr.left)}{build_f_string_parts(sub_expr.right)}"
                    elif isinstance(sub_expr, ast.Literal) and isinstance(sub_expr.value, str):
                        return sub_expr.value.replace("{", "{{").replace("}", "}}")
                    else:
                        return f"{{{self._visit(sub_expr)}}}"
                f_string_content = build_f_string_parts(expr.arguments[0])
                return f'print(f"{f_string_content}")'
            return f"print({', '.join(args)})"

        if func_name == "panjang" and len(args) == 1:
            return f"len({args[0]})"

        if func_name == "tambah" and len(args) == 2:
            return f"{args[0]}.append({args[1]})"

        return f"{func_name}({', '.join(args)})"
