from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Any, Optional
from .token import Token

class ASTNode(ABC):
    @abstractmethod
    def accept(self, visitor): pass
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
