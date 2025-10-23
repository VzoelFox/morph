# interpreter/ast_nodes.py
from dataclasses import dataclass
from typing import List, Any
# (definisi kelas ASTNode, Statement, Expression, Literal, Variable, dll. disingkat)
from .token import Token
@dataclass
class AturStatement:
    name: Token
    initializer: Any
@dataclass
class LihatStatement:
    expression: Any
# ...dan seterusnya
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Any, Optional
from .token import Token

class ASTNode(ABC):
    @abstractmethod
    def accept(self, visitor): pass

class Statement(ASTNode): pass
class Expression(ASTNode): pass

# --- Expressions ---
class Statement(ASTNode): pass
class Expression(ASTNode): pass

@dataclass
class Literal(Expression):
    value: Any
    def accept(self, visitor): return visitor.visit_Literal(self)
@dataclass
class Variable(Expression):
    name: Token
    def accept(self, visitor): return visitor.visit_Variable(self)
@dataclass
class UnaryExpression(Expression):
    operator: Token
    right: Expression
    def accept(self, visitor): return visitor.visit_UnaryExpression(self)
@dataclass
class BinaryExpression(Expression):
    left: Expression
    operator: Token
    right: Expression
    def accept(self, visitor): return visitor.visit_BinaryExpression(self)
@dataclass
class FunctionCall(Expression):
    callee: Expression
    arguments: List[Expression]
    def accept(self, visitor): return visitor.visit_FunctionCall(self)
@dataclass
class AmbilExpression(Expression):
    keyword: Token
    path: Expression
    def accept(self, visitor): return visitor.visit_AmbilExpression(self)
@dataclass
class GetExpression(Expression):
    objek: Expression
    name: Token
    def accept(self, visitor): return visitor.visit_GetExpression(self)
@dataclass
class AmbilExpression(Expression):
    path: Expression
    def accept(self, visitor): return visitor.visit_AmbilExpression(self)

# --- Statements ---
class BinaryExpression(Expression):
    left: Expression
    operator: Token
    right: Expression
    def accept(self, visitor): return visitor.visit_BinaryExpression(self)

@dataclass
class Program(ASTNode):
    statements: List[Statement]
    def accept(self, visitor): return visitor.visit_Program(self)
@dataclass
class ExpressionStatement(Statement):
    expression: Expression
    def accept(self, visitor): return visitor.visit_ExpressionStatement(self)
@dataclass
class AturStatement(Statement):
    name: Token
    initializer: Expression
    fallback: Optional['AturStatement'] = None
    def accept(self, visitor): return visitor.visit_AturStatement(self)
@dataclass
class BlokStatement(Statement):
    statements: List[Statement]
    def accept(self, visitor): return visitor.visit_BlokStatement(self)
@dataclass
class ProsesStatement(Statement):
    name: Token
    params: List[Token]
    body: BlokStatement
    def accept(self, visitor): return visitor.visit_ProsesStatement(self)
@dataclass
class KembaliStatement(Statement):
    keyword: Token
    value: Optional[Expression]
    def accept(self, visitor): return visitor.visit_KembaliStatement(self)
@dataclass
class ManagementStatement(Statement):
    body: BlokStatement
    def accept(self, visitor): return visitor.visit_ManagementStatement(self)

@dataclass
class JikaStatement(Statement):
    condition: Expression
    then_branch: Statement
    # else_branch opsional
    def accept(self, visitor): return visitor.visit_JikaStatement(self)
class AturStatement(Statement):
    name: Token
    initializer: Expression
    fallback: Optional['AturStatement'] = None
    def accept(self, visitor): return visitor.visit_AturStatement(self)
@dataclass
class ExpressionStatement(Statement):
from typing import List, Any
from .token import Token

# --- Kelas Dasar ---
class ASTNode(ABC):
    """Kelas dasar untuk semua simpul AST."""
    @abstractmethod
    def accept(self, visitor):
        pass
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
