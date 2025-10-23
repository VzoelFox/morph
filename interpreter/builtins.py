# interpreter/builtins.py
from .callable import VzoelCallable
from typing import List, Any

class UbahKeString(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments: List[Any]) -> Any:
        return str(arguments[0])

class Panjang(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], list):
            # Di masa depan, kita bisa melempar VzoelRuntimeException di sini
            return 0
        return len(arguments[0])

class Tambah(VzoelCallable):
    def arity(self) -> int:
        return 2

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], list):
            # Penanganan error
            return None
        arguments[0].append(arguments[1])
        return arguments[0]
