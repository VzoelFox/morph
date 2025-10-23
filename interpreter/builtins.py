# interpreter/builtins.py
from .callable import VzoelCallable
from typing import List, Any

class Panjang(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], list):
            return 0
        return len(arguments[0])

class Tambah(VzoelCallable):
    def arity(self) -> int:
        return 2

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], list):
            return None
        arguments[0].append(arguments[1])
        return arguments[0]
