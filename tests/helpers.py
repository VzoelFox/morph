from io import StringIO
import sys
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter

def run_vzoel_code_capture_output(source: str) -> list[str]:
    original_stdout = sys.stdout
    captured_output = StringIO()
    sys.stdout = captured_output

    try:
        lexer = Lexer(source)
        tokens = lexer.scan_tokens()
        parser = Parser(tokens)
        program = parser.parse()
        # Penting: periksa error parser di sini
        if parser.errors:
            raise Exception(f"Parser errors: {[str(e) for e in parser.errors]}")

        interpreter = Interpreter()
        interpreter.interpret(program)

    except Exception as e:
        sys.stdout = original_stdout
        print("--- CAPTURED STDOUT ---")
        print(captured_output.getvalue())
        print("--- EXCEPTION ---")
        raise e
    finally:
        sys.stdout = original_stdout

    output_lines = captured_output.getvalue().strip().split('\n')
    return [line for line in output_lines if line]
