"""
Modul ini berisi Compiler, yang mengubah Abstract Syntax Tree (AST)
menjadi Intermediate Representation (IR) linear.
"""

from interpreter import ast_nodes as ast
from compiler import instructions

class Compiler: # Menghapus pewarisan dari ast.Visitor
    def __init__(self):
        self.instructions = []
        self._temp_counter = 0

    def _new_temp(self):
        """Menghasilkan nama temporary baru, misalnya 't1', 't2', dst."""
        self._temp_counter += 1
        return f"t{self._temp_counter}"

    def compile(self, program: ast.Program):
        """
        Menerjemahkan seluruh program AST menjadi daftar instruksi IR.
        """
        self.instructions = []
        for statement in program.statements:
            statement.accept(self)
        return self.instructions

    # --- Metode Visitor (akan diimplementasikan secara bertahap) ---

    def visit_Literal(self, node: ast.Literal):
        """Mengunjungi node Literal."""
        temp_dest = self._new_temp()
        instruction = instructions.LoadConst(value=node.value, dest=temp_dest)
        self.instructions.append(instruction)
        return temp_dest # Mengembalikan nama temporary untuk digunakan oleh ekspresi yang lebih tinggi

    def visit_ExpressionStatement(self, node: ast.ExpressionStatement):
        """Mengunjungi node ExpressionStatement."""
        node.expression.accept(self)
        # Di masa depan, mungkin perlu menangani hasil ekspresi yang tidak digunakan.
