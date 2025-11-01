# interpreter/builtins.py
import math
from .callable import VzoelCallable
from typing import List, Any
from .errors import VzoelRuntimeException
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

class Potong(VzoelCallable):
    def arity(self) -> int:
        return 3

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], str):
            raise VzoelRuntimeException(None, "Argumen pertama untuk 'potong' harus berupa string.")

        if not isinstance(arguments[1], (int, float)) or not isinstance(arguments[2], (int, float)):
            raise VzoelRuntimeException(None, "Argumen kedua dan ketiga untuk 'potong' harus berupa angka.")

        text = arguments[0]
        start = int(arguments[1])
        end = int(arguments[2])
        return text[start:end]

class Akar(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], (int, float)):
            raise VzoelRuntimeException(None, "Argumen untuk 'akar' harus berupa angka.")
        return math.sqrt(arguments[0])

class Pangkat(VzoelCallable):
    def arity(self) -> int:
        return 2

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], (int, float)) or not isinstance(arguments[1], (int, float)):
            raise VzoelRuntimeException(None, "Argumen untuk 'pangkat' harus berupa angka.")
        return math.pow(arguments[0], arguments[1])
