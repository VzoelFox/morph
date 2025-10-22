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
