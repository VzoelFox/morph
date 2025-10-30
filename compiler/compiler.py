"""
Modul ini berisi Compiler, yang mengubah Abstract Syntax Tree (AST)
menjadi Intermediate Representation (IR) linear.
"""

from interpreter import ast_nodes as ast
from compiler import instructions
from interpreter.token_types import TokenType


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

    def visit_UnaryExpression(self, node: ast.UnaryExpression):
        """Mengunjungi node UnaryExpression."""
        operand_temp = node.right.accept(self)

        dest_temp = self._new_temp()

        if node.operator.type == TokenType.MINUS:
            instruction = instructions.Negate(operand=operand_temp, dest=dest_temp)
            self.instructions.append(instruction)
        else:
            # Di masa depan, bisa menangani operator unary lain seperti '!'
            raise NotImplementedError(f"Operator unary {node.operator.type} belum didukung.")

        return dest_temp

    def visit_BinaryExpression(self, node: ast.BinaryExpression):
        """Mengunjungi node BinaryExpression."""
        left_temp = node.left.accept(self)
        right_temp = node.right.accept(self)

        dest_temp = self._new_temp()

        op_map = {
            TokenType.PLUS: instructions.Add,
            TokenType.MINUS: instructions.Sub,
            TokenType.BINTANG: instructions.Mul,
            TokenType.GARIS_MIRING: instructions.Div,
            TokenType.LEBIH_DARI: instructions.GreaterThan,
            TokenType.KURANG_DARI: instructions.LessThan,
        }

        instruction_class = op_map.get(node.operator.type)

        if instruction_class:
            instruction = instruction_class(left=left_temp, right=right_temp, dest=dest_temp)
            self.instructions.append(instruction)
        else:
            raise NotImplementedError(f"Operator biner {node.operator.type} belum didukung.")

        return dest_temp

    def visit_AturStatement(self, node: ast.AturStatement):
        """Mengunjungi node AturStatement."""
        src_temp = node.initializer.accept(self)

        instruction = instructions.StoreVar(name=node.name.literal, src=src_temp)
        self.instructions.append(instruction)

    def visit_Variable(self, node: ast.Variable):
        """Mengunjungi node Variable."""
        dest_temp = self._new_temp()
        instruction = instructions.LoadVar(name=node.name.literal, dest=dest_temp)
        self.instructions.append(instruction)
        return dest_temp
