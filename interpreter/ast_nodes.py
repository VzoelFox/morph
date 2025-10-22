from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Any
from .token import Token

# --- Kelas Dasar ---
class ASTNode(ABC):
    """Kelas dasar untuk semua simpul AST."""
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

@dataclass
class Variable(Expression):
    """Merepresentasikan nama variabel, misal: 'pesan'."""
    name: Token

@dataclass
class FunctionCall(Expression):
    """Merepresentasikan pemanggilan fungsi/proses, misal: lihat(pesan)."""
    callee: Expression
    arguments: List[Expression]


# --- Simpul Pernyataan (Statements) ---

@dataclass
class Program(ASTNode):
    """Simpul akar dari AST, berisi daftar semua pernyataan."""
    statements: List[Statement]

@dataclass
class AturStatement(Statement):
    """Merepresentasikan pernyataan 'atur', misal: atur pesan = "Halo"."""
    name: Token
    initializer: Expression

@dataclass
class ExpressionStatement(Statement):
    """
    Pernyataan yang hanya terdiri dari satu ekspresi.
    Berguna untuk membungkus pemanggilan fungsi seperti `lihat(pesan)`.
    """
    expression: Expression
