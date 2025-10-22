import io, sys
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter

def run_vzoel_code(source_code: str) -> str:
    original_stdout = sys.stdout
    sys.stdout = captured_output = io.StringIO()
    try:
        # Penuh pipeline
        lexer = Lexer(source_code)
        tokens = lexer.scan_tokens()
        parser = Parser(tokens)
        statements = parser.parse().statements
        interpreter = Interpreter()
        interpreter.interpret(statements)
    finally:
        sys.stdout = original_stdout
    return captured_output.getvalue()
