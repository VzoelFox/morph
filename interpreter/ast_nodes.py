from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Any
from .token import Token

# --- Kelas Dasar ---
class ASTNode(ABC):
    """Kelas dasar untuk semua simpul AST."""
    @abstractmethod
    def accept(self, visitor):
        pass

class Statement(ASTNode):
    """Kelas dasar untuk pernyataan (statements)."""
    pass

class Expression(ASTNode):
    """Kelas dasar untuk ekspresi (expressions)."""
    pass

# --- Simpul Ekspresi (Expressions) ---

@dataclass
class Literal(Expression):
    """Merepresentasikan nilai literal seperti "string", 123, benar, salah."""
    value: Any
    def accept(self, visitor): return visitor.visit_Literal(self)

@dataclass
class Variable(Expression):
    """Merepresentasikan nama variabel, misal: 'pesan'."""
    name: Token
    def accept(self, visitor): return visitor.visit_Variable(self)

@dataclass
class FunctionCall(Expression):
    """Merepresentasikan pemanggilan fungsi/proses, misal: lihat(pesan)."""
    callee: Expression
    arguments: List[Expression]
    def accept(self, visitor): return visitor.visit_FunctionCall(self)

@dataclass
class AmbilExpression(Expression):
    """Merepresentasikan ekspresi 'ambil', misal: ambil dari "path/modul.vz"."""
    keyword: Token # Token 'ambil' untuk error reporting
    path: Expression # Seharusnya berupa Literal String
    def accept(self, visitor): return visitor.visit_AmbilExpression(self)

@dataclass
class GetExpression(Expression):
    """Merepresentasikan akses properti, misal: modul.properti."""
    objek: Expression
    name: Token # Nama properti
    def accept(self, visitor): return visitor.visit_GetExpression(self)


# --- Simpul Pernyataan (Statements) ---

@dataclass
class Program(ASTNode):
    """Simpul akar dari AST, berisi daftar semua pernyataan."""
    statements: List[Statement]
    def accept(self, visitor): return visitor.visit_Program(self)

from typing import Optional

@dataclass
class AturStatement(Statement):
    """Merepresentasikan pernyataan 'atur', misal: atur pesan = "Halo"."""
    name: Token
    initializer: Expression
    fallback: Optional['AturStatement'] = None # Untuk 'jika ... tidak ... maka atur'
    def accept(self, visitor): return visitor.visit_AturStatement(self)

@dataclass
class ExpressionStatement(Statement):
    """
    Pernyataan yang hanya terdiri dari satu ekspresi.
    Berguna untuk membungkus pemanggilan fungsi seperti `lihat(pesan)`.
    """
    expression: Expression
    def accept(self, visitor): return visitor.visit_ExpressionStatement(self)
