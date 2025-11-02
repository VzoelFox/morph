from io import StringIO
import sys
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter

def run_vzoel_code_capture_output(source: str) -> list[str]:
    original_stdout = sys.stdout
    captured_output = StringIO()
    sys.stdout = captured_output

    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()
    interpreter = Interpreter()
    interpreter.interpret(program)

    sys.stdout = original_stdout
    output_lines = captured_output.getvalue().strip().split('\n')
    return [line for line in output_lines if line]

def run_vzoel_code(source: str):
    """Menjalankan kode Vzoel, berguna untuk tes yang diharapkan gagal."""
    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()
    interpreter = Interpreter()
    interpreter.interpret(program)
