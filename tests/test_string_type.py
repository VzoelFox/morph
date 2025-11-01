# tests/test_string_type.py
import pytest
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter
from interpreter.errors import VzoelRuntimeException
from unittest.mock import patch
import io

def run_code(source_code):
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()
    assert not parser.errors

    with patch('sys.stdout', new=io.StringIO()) as fake_stdout:
        interpreter = Interpreter()
        interpreter.interpret(program)
        return fake_stdout.getvalue().strip()

def test_string_literal_evaluation():
    output = run_code('lihat("hello, world!")')
    assert output == "hello, world!"

def test_string_assignment():
    source = """
    atur pesan = "ini adalah pesan"
    lihat(pesan)
    """
    output = run_code(source)
    assert output == "ini adalah pesan"

def test_empty_string():
    output = run_code('lihat("")')
    assert output == ""

def test_string_with_numbers_and_symbols():
    source = 'lihat("Vzoel Word v0.3!@#")'
    output = run_code(source)
    assert output == "Vzoel Word v0.3!@#"

def test_string_slicing_with_potong():
    source = """
    atur teks = "halo dunia"
    atur hasil = potong(teks, 0, 4)
    lihat(hasil)
    """
    output = run_code(source)
    assert output == "halo"

def test_potong_with_invalid_arguments():
    source_1 = 'potong(123, 0, 1)'
    with pytest.raises(VzoelRuntimeException) as e:
        run_code(source_1)
    assert "Argumen pertama untuk 'potong' harus berupa string." in e.value.message

    source_2 = 'potong("hello", "a", "b")'
    with pytest.raises(VzoelRuntimeException) as e:
        run_code(source_2)
    assert "Argumen kedua dan ketiga untuk 'potong' harus berupa angka." in e.value.message

def test_ke_kecil_function():
    source = 'lihat(ke_kecil("HALO DUNIA"))'
    output = run_code(source)
    assert output == "halo dunia"

def test_ke_besar_function():
    source = 'lihat(ke_besar("halo dunia"))'
    output = run_code(source)
    assert output == "HALO DUNIA"

def test_case_conversion_error_handling():
    source_1 = 'ke_kecil(123)'
    with pytest.raises(VzoelRuntimeException) as e:
        run_code(source_1)
    assert "Argumen untuk 'ke_kecil' harus berupa string." in e.value.message

    source_2 = 'ke_besar([1, 2])'
    with pytest.raises(VzoelRuntimeException) as e:
        run_code(source_2)
    assert "Argumen untuk 'ke_besar' harus berupa string." in e.value.message
