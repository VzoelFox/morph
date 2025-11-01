# interpreter/ast_nodes.py
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Any, Optional
from .token import Token

# --- Kelas Dasar ---
class ASTNode(ABC):
    """Kelas dasar untuk semua simpul AST."""
    @abstractmethod
    def accept(self, visitor):
        pass

class Visitor(ABC):
    # Expressions
    @abstractmethod
    def visit_Literal(self, expr: 'Literal'): pass
    @abstractmethod
    def visit_Variable(self, expr: 'Variable'): pass
    @abstractmethod
    def visit_FunctionCall(self, expr: 'FunctionCall'): pass
    @abstractmethod
    def visit_AmbilExpression(self, expr: 'AmbilExpression'): pass
    @abstractmethod
    def visit_GetExpression(self, expr: 'GetExpression'): pass
    @abstractmethod
    def visit_SubscriptExpression(self, expr: 'SubscriptExpression'): pass
    @abstractmethod
    def visit_BinaryExpression(self, expr: 'BinaryExpression'): pass
    @abstractmethod
    def visit_UnaryExpression(self, expr: 'UnaryExpression'): pass
    @abstractmethod
    def visit_Grouping(self, expr: 'Grouping'): pass
    @abstractmethod
    def visit_ListLiteral(self, expr: 'ListLiteral'): pass
    @abstractmethod
    def visit_MapLiteral(self, expr: 'MapLiteral'): pass
    # Statements
    @abstractmethod
    def visit_BlokStatement(self, stmt: 'BlokStatement'): pass
    @abstractmethod
    def visit_ProsesStatement(self, stmt: 'ProsesStatement'): pass
    @abstractmethod
    def visit_KembaliStatement(self, stmt: 'KembaliStatement'): pass
    @abstractmethod
    def visit_AturStatement(self, stmt: 'AturStatement'): pass
    @abstractmethod
    def visit_ExpressionStatement(self, stmt: 'ExpressionStatement'): pass
    @abstractmethod
    def visit_UlangiStatement(self, stmt: 'UlangiStatement'): pass
    @abstractmethod
    def visit_ManagementDeclaration(self, stmt: 'ManagementDeclaration'): pass
    @abstractmethod
    def visit_JalankanStatement(self, stmt: 'JalankanStatement'): pass

class Statement(ASTNode):
    """Kelas dasar untuk pernyataan (statements)."""
    pass

class Expression(ASTNode):
    """Kelas dasar untuk ekspresi (expressions)."""
    pass

# --- Simpul Ekspresi (Expressions) ---
@dataclass
class Literal(Expression):
    value: Any
    def accept(self, visitor): return visitor.visit_Literal(self)

@dataclass
class Variable(Expression):
    name: Token
    def accept(self, visitor): return visitor.visit_Variable(self)

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
class SubscriptExpression(Expression):
    objek: Expression
    indeks: Expression
    def accept(self, visitor): return visitor.visit_SubscriptExpression(self)

@dataclass
class BinaryExpression(Expression):
    left: Expression
    operator: Token
    right: Expression
    def accept(self, visitor): return visitor.visit_BinaryExpression(self)

@dataclass
class UnaryExpression(Expression):
    operator: Token
    right: Expression
    def accept(self, visitor): return visitor.visit_UnaryExpression(self)

@dataclass
class Grouping(Expression):
    expression: Expression
    def accept(self, visitor): return visitor.visit_Grouping(self)

@dataclass
class ListLiteral(Expression):
    elements: List[Expression]
    def accept(self, visitor): return visitor.visit_ListLiteral(self)

@dataclass
class MapLiteral(Expression):
    keys: List[Expression]
    values: List[Expression]
    def accept(self, visitor): return visitor.visit_MapLiteral(self)

# --- Simpul Pernyataan (Statements) ---
@dataclass
class Program(ASTNode):
    statements: List[Statement]
    def accept(self, visitor): return visitor.visit_Program(self)

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
class AturStatement(Statement):
    name: Token
    initializer: Expression
    fallback: Optional['AturStatement'] = None
    def accept(self, visitor): return visitor.visit_AturStatement(self)

@dataclass
class ExpressionStatement(Statement):
    expression: Expression
    def accept(self, visitor): return visitor.visit_ExpressionStatement(self)

@dataclass
class UlangiStatement(Statement):
    count: Expression
    body: Statement
    def accept(self, visitor): return visitor.visit_UlangiStatement(self)

@dataclass
class ManagementDeclaration(Statement):
    name: Token
    bagian_list: List['BagianDeclaration']
    def accept(self, visitor):
        return visitor.visit_ManagementDeclaration(self)

@dataclass
class BagianDeclaration(ASTNode):
    name: Token
    pecahan_list: List['PecahanDeclaration']
    def accept(self, visitor):
        return visitor.visit_BagianDeclaration(self)

@dataclass
class PecahanDeclaration(ASTNode):
    name: Token
    body: BlokStatement
    def accept(self, visitor):
        return visitor.visit_PecahanDeclaration(self)

@dataclass
class JalankanStatement(Statement):
    management_name: Token
    def accept(self, visitor):
        return visitor.visit_JalankanStatement(self)
