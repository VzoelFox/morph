# tests/helpers.py
from io import StringIO
import sys
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter

def run_vzoel_code_capture_output(source: str) -> list[str]:
    """Menjalankan kode Vzoel dan mengembalikan output dari 'lihat' sebagai list of strings."""
    # Simpan stdout asli
    original_stdout = sys.stdout
    # Alihkan stdout ke StringIO
    captured_output = StringIO()
    sys.stdout = captured_output

    # Inisialisasi dan jalankan interpreter
    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()
    interpreter = Interpreter()
    interpreter.interpret(program)

    # Pulihkan stdout
    sys.stdout = original_stdout
    # Ambil output dan pisahkan berdasarkan baris
    output_lines = captured_output.getvalue().strip().split('\n')
    return [line for line in output_lines if line]
