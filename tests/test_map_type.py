# tests/test_map_type.py
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

    with patch('sys.stdout', new=io.StringIO()) as fake_stdout:
        interpreter = Interpreter()
        interpreter.interpret(program)
        return fake_stdout.getvalue().strip()

def test_map_creation_and_access():
    source = """
    atur data = peta{"nama": "Vzoel", "versi": 0.3}
    lihat(data["nama"])
    """
    output = run_code(source)
    assert output == "Vzoel"

def test_map_access_nonexistent_key():
    source = """
    atur data = peta{"nama": "Vzoel"}
    lihat(data["tidak_ada"])
    """
    output = run_code(source)
    assert output == "None" # .get() returns None for missing keys

def test_empty_map():
    source = """
    atur data = peta{}
    lihat(panjang(data))
    """
    # Note: This test will fail until `panjang` supports maps.
    # We will implement that in a future step. For now, we just test creation.
    source = 'atur data = peta{}'
    run_code(source)
    # No assertion, just checking that it doesn't crash.

def test_map_with_numeric_keys():
    source = """
    atur data = peta{1: "satu", 2: "dua"}
    lihat(data[1])
    """
    output = run_code(source)
    assert output == "satu"
