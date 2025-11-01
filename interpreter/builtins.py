# interpreter/builtins.py
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

class KeKecil(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], str):
            raise VzoelRuntimeException(None, "Argumen untuk 'ke_kecil' harus berupa string.")
        return arguments[0].lower()

class KeBesar(VzoelCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter, arguments: List[Any]) -> Any:
        if not isinstance(arguments[0], str):
            raise VzoelRuntimeException(None, "Argumen untuk 'ke_besar' harus berupa string.")
        return arguments[0].upper()
