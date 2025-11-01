from dataclasses import dataclass
from .token_types import TokenType

@dataclass
class Token:
    type: TokenType
    literal: str
    line: int = 1
    column: int = 1
