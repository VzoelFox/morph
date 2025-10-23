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
from typing import Dict, Any
from .token import Token

class Environment:
    def __init__(self):
        self.values: Dict[str, Any] = {}

    def define(self, name: str, value: Any):
        """Mendefinisikan variabel baru."""
        self.values[name] = value

    def get(self, name_token: Token) -> Any:
        """Mengambil nilai variabel."""
        name = name_token.literal
        if name in self.values:
            return self.values[name]

        # Nanti kita bisa buat error handling yang lebih baik
        raise Exception(f"Variabel '{name}' tidak terdefinisi.")
