# tests/test_string_type.py
import pytest
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter
from unittest.mock import patch
import io

def run_code(source_code):
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()

    # Redirect stdout to capture 'lihat' output
    with patch('sys.stdout', new=io.StringIO()) as fake_stdout:
        interpreter = Interpreter()
        interpreter.interpret(program)
        return fake_stdout.getvalue().strip()

def test_string_literal_evaluation():
    # Test bahwa string literal dievaluasi dengan benar
    output = run_code('lihat("hello, world!")')
    assert output == "hello, world!"

def test_string_assignment():
    # Test bahwa string dapat di-assign ke variabel dan digunakan
    source = """
    atur pesan = "ini adalah pesan"
    lihat(pesan)
    """
    output = run_code(source)
    assert output == "ini adalah pesan"

def test_empty_string():
    # Test string kosong
    output = run_code('lihat("")')
    assert output == ""

def test_string_with_numbers_and_symbols():
    # Test string yang mengandung angka dan simbol
    source = 'lihat("Vzoel Word v0.3!@#")'
    output = run_code(source)
    assert output == "Vzoel Word v0.3!@#"
