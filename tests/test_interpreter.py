import pytest
from pathlib import Path
import io, sys
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter

def run_vzoel_code(source_code: str) -> str:
    original_stdout = sys.stdout
    sys.stdout = captured_output = io.StringIO()
    try:
        lexer = Lexer(source_code)
        tokens = lexer.scan_tokens()
        parser = Parser(tokens)
        program_ast = parser.parse()
        interpreter = Interpreter()
        interpreter.interpret(program_ast)
    finally:
        sys.stdout = original_stdout
    return captured_output.getvalue()

def test_hello_world_interpreter():
    source_path = Path(__file__).parent.parent / "examples" / "hello.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    output = run_vzoel_code(source_code)
    expected_output = "Halo Dunia dari Vzoel Word v0.2!\n"
    assert output == expected_output
