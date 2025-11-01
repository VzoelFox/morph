# interpreter/environment.py
from typing import Dict, Any, Optional
from .token import Token
from .errors import VzoelRuntimeException

class Environment:
    def __init__(self, enclosing: Optional['Environment'] = None):
        self.values: Dict[str, Any] = {}
        self.enclosing = enclosing

    def define(self, name: str, value: Any):
        self.values[name] = value

    def get(self, name_token: Token) -> Any:
        name = name_token.literal
        if name in self.values:
            return self.values[name]
        if self.enclosing is not None:
            return self.enclosing.get(name_token)
        raise VzoelRuntimeException(name_token, f"Variabel '{name}' tidak terdefinisi.")
