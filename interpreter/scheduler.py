# interpreter/scheduler.py
from collections import deque
import time
from typing import Generator, Any, Optional, TYPE_CHECKING

if TYPE_CHECKING:
    from .interpreter import Interpreter # Hanya untuk type checking

from .callable import VzoelFunction

class Task:
    def __init__(self, target: VzoelFunction, args: list, interpreter: 'Interpreter'):
        self.target = target
        self.args = args
        self.interpreter = interpreter
        self._coroutine: Optional[Generator[Any, None, Any]] = None

    def run(self) -> Any:
        if self._coroutine is None:
            self._coroutine = self.target.call(self.interpreter, self.args)
        # Lanjutkan ke yield berikutnya. Akan memunculkan StopIteration jika selesai.
        return next(self._coroutine)

class Scheduler:
    def __init__(self):
        self.ready = deque()
        self.sleeping = []

    def add_task(self, task: Task):
        self.ready.append(task)

    def run_until_complete(self):
        while self.ready or self.sleeping:
            now = time.time()
            # Pindahkan task yang sudah 'bangun' dari sleeping ke ready
            ready_to_wake = [t for t in self.sleeping if t[0] <= now]
            self.sleeping = [t for t in self.sleeping if t[0] > now]
            for _, task in ready_to_wake:
                self.ready.append(task)

            if not self.ready:
                if self.sleeping:
                    time.sleep(0.001) # Tidur sebentar jika hanya ada task yang sedang tidur
                continue

            task = self.ready.popleft()
            try:
                # Lanjutkan eksekusi task
                result = task.run()
                # Jika task menghasilkan sesuatu, itu adalah durasi tidur
                wake_time = time.time() + result
                self.sleeping.append((wake_time, task))
            except StopIteration:
                # Task selesai, jangan jadwalkan lagi
                pass
