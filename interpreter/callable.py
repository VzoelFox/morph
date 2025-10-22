from abc import ABC, abstractmethod
from typing import List, Any
import interpreter.ast_nodes as ast
from .environment import Environment
from .errors import Return

class VzoelCallable(ABC):
    @abstractmethod
    def arity(self) -> int: pass
    @abstractmethod
    def call(self, interpreter, arguments: List[Any]) -> Any: pass

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
        try:
            interpreter.execute_block(self.declaration.body.statements, environment)
        except Return as returnValue:
            return returnValue.value
        return None

class VzoelCallable(ABC):
    @abstractmethod
    def arity(self) -> int:
        """Mengembalikan jumlah argumen yang diharapkan."""
        pass

    @abstractmethod
    def call(self, interpreter, arguments: List[Any]) -> Any:
        """Mengeksekusi fungsi."""
        pass
