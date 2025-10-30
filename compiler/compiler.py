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
        self._label_counter = 0

    def _new_temp(self):
        """Menghasilkan nama temporary baru, misalnya 't1', 't2', dst."""
        self._temp_counter += 1
        return f"t{self._temp_counter}"

    def _new_label(self, name):
        """Menghasilkan nama label unik."""
        self._label_counter += 1
        return f"{name}_{self._label_counter}"

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

    def visit_UlangiStatement(self, node: ast.UlangiStatement):
        """Mengunjungi node UlangiStatement."""
        # 1. Kompilasi ekspresi jumlah perulangan
        count_temp = node.count.accept(self)

        # 2. Siapkan label untuk awal dan akhir perulangan
        start_label_name = self._new_label("start_loop")
        end_label_name = self._new_label("end_loop")

        # 3. Buat variabel counter internal
        counter_name = f"__counter_{self._label_counter}" # Kaitkan dengan label unik

        # 4. Inisialisasi counter ke 0
        zero_temp = self._new_temp()
        self.instructions.append(instructions.LoadConst(value=0.0, dest=zero_temp))
        self.instructions.append(instructions.StoreVar(name=counter_name, src=zero_temp))

        # 5. Tambahkan label awal perulangan
        self.instructions.append(instructions.Label(name=start_label_name))

        # 6. Kondisi perulangan: counter < jumlah
        counter_val_temp = self._new_temp()
        self.instructions.append(instructions.LoadVar(name=counter_name, dest=counter_val_temp))

        condition_temp = self._new_temp()
        self.instructions.append(instructions.LessThan(left=counter_val_temp, right=count_temp, dest=condition_temp))

        # 7. Jika kondisi salah, lompat ke akhir perulangan
        self.instructions.append(instructions.JumpIfFalse(condition=condition_temp, target=end_label_name))

        # 8. Kompilasi badan perulangan
        node.body.accept(self)

        # 9. Naikkan counter (counter = counter + 1)
        one_temp = self._new_temp()
        self.instructions.append(instructions.LoadConst(value=1.0, dest=one_temp))

        new_counter_val_temp = self._new_temp()
        self.instructions.append(instructions.Add(left=counter_val_temp, right=one_temp, dest=new_counter_val_temp))
        self.instructions.append(instructions.StoreVar(name=counter_name, src=new_counter_val_temp))

        # 10. Lompat kembali ke awal perulangan
        self.instructions.append(instructions.Jump(target=start_label_name))

        # 11. Tambahkan label akhir perulangan
        self.instructions.append(instructions.Label(name=end_label_name))
