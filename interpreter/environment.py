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
