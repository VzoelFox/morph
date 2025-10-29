# tests/test_analyzer.py
import pytest
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from analyzer.static_analyzer import StaticAnalyzer
from interpreter.errors import VzoelRuntimeException

def run_analyzer(source_code: str) -> bool:
    """
    Helper untuk menjalankan kode melalui Lexer, Parser, dan StaticAnalyzer.
    Mengembalikan True jika analisis berhasil, False jika gagal.
    """
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()

    if parser.errors:
        for error in parser.errors:
            print(f"Parser Error: {error}") # Untuk debug
        return False

    analyzer = StaticAnalyzer()
    return analyzer.analyze(program)

# --- Tes untuk analisis yang seharusnya BERHASIL ---

def test_valid_variable_declaration():
    code = """
    atur a = 10
    atur b = "halo"
    atur c = benar
    atur d = a + 5
    atur e = b + " dunia"
    """
    assert run_analyzer(code) is True

def test_valid_numeric_operations():
    code = "atur x = 10 * (5 - 2) / 3"
    assert run_analyzer(code) is True

def test_valid_string_concatenation():
    code = 'atur s = "a" + "b" + "c"'
    assert run_analyzer(code) is True

def test_valid_comparisons():
    code = """
    atur a = 5 > 2
    atur b = "x" == "x"
    atur c = 10 != 20
    """
    assert run_analyzer(code) is True

# --- Tes untuk analisis yang seharusnya GAGAL ---

def test_invalid_addition_number_and_string():
    code = "atur x = 10 + 'ini string'"
    assert run_analyzer(code) is False

def test_invalid_subtraction_with_string():
    code = 'atur x = "test" - 5'
    assert run_analyzer(code) is False

def test_undefined_variable():
    code = "atur x = y + 5"
    assert run_analyzer(code) is False

def test_type_mismatch_in_binary_op():
    code = """
    atur angka = 10
    atur hasil = angka + "ini string"
    """
    assert run_analyzer(code) is False
