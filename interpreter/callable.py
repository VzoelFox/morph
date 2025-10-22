from abc import ABC, abstractmethod
from typing import List, Any

class VzoelCallable(ABC):
    @abstractmethod
    def arity(self) -> int:
        """Mengembalikan jumlah argumen yang diharapkan."""
        pass

    @abstractmethod
    def call(self, interpreter, arguments: List[Any]) -> Any:
        """Mengeksekusi fungsi."""
        pass
