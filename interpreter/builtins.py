# interpreter/builtins.py
from .callable import VzoelCallable
from typing import List, Any

class UbahKeString(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments: List[Any]) -> Any:
        return str(arguments[0])
