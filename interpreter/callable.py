# interpreter/callable.py
from abc import ABC, abstractmethod
from typing import List, Any
import interpreter.ast_nodes as ast
from .environment import Environment
from .errors import Return

class VzoelCallable(ABC):
    @abstractmethod
    def arity(self) -> int:
        """Mengembalikan jumlah argumen yang diharapkan."""
        pass

    @abstractmethod
    def call(self, interpreter, arguments: List[Any]) -> Any:
        """Mengeksekusi fungsi."""
        pass

class Lihat(VzoelCallable):
    def arity(self) -> int: return 1
    def call(self, interpreter, arguments: List[Any]): print(arguments[0]); return None

class VzoelFunction(VzoelCallable):
    def __init__(self, declaration: ast.ProsesStatement, closure: Environment):
        self.declaration = declaration
        self.closure = closure

    def arity(self) -> int:
        return len(self.declaration.params)

    def call(self, interpreter, arguments: List[Any]) -> Any:
        environment = Environment(enclosing=self.closure)
        for i, param in enumerate(self.declaration.params):
            environment.define(param.literal, arguments[i])

        # Ini sekarang adalah sebuah generator
        block_executor = interpreter.execute_block(self.declaration.body.statements, environment)

        while True:
            try:
                # Lanjutkan eksekusi sampai yield (tunggu) berikutnya
                yield next(block_executor)
            except StopIteration:
                # Selesai tanpa nilai kembali eksplisit
                break
            except Return as r:
                # Selesai dengan nilai kembali
                return r.value
        return None
