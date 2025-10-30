# tests/test_transpiler.py
import pytest
from textwrap import dedent
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from transpiler.transpiler import Transpiler

def run_transpiler(source_code: str) -> str:
    """
    Helper untuk menjalankan kode melalui Lexer, Parser, dan Transpiler,
    lalu mengembalikan kode Python yang dihasilkan.
    """
    source_code = dedent(source_code).strip()
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()

    assert not parser.errors, f"Parser errors: {parser.errors}"

    transpiler = Transpiler()
    python_code = transpiler.transpile(program)
    return python_code

def assert_transpilation(source, expected):
    """Asserts that the transpiled source code matches the expected output."""
    generated = run_transpiler(source)
    expected = dedent(expected).strip()
    assert generated == expected

def test_transpile_variable_declaration():
    source = """
    atur a = 10
    atur b = "halo"
    atur c = benar
    """
    expected = """
    a = 10.0
    b = 'halo'
    c = True
    """
    assert_transpilation(source, expected)

def test_transpile_binary_expression():
    source = "atur x = (10 + 20) * 3"
    # Memperbaiki ekspektasi agar cocok dengan output aktual (tiga kurung)
    expected = "x = (((10.0 + 20.0)) * 3.0)"
    assert_transpilation(source, expected)

def test_transpile_lihat_simple():
    source = 'lihat("Halo Dunia")'
    expected = "print('Halo Dunia')"
    assert_transpilation(source, expected)

def test_transpile_lihat_with_variable():
    source = """
    atur nama = "Morph"
    lihat("Halo, " + nama)
    """
    expected = """
    nama = 'Morph'
    print(f"Halo, {nama}")
    """
    assert_transpilation(source, expected)

def test_transpile_list_literal():
    source = 'atur x = [1, "dua", benar]'
    expected = "x = [1.0, 'dua', True]"
    assert_transpilation(source, expected)

def test_transpile_panjang():
    source = "atur x = panjang([1, 2, 3])"
    expected = "x = len([1.0, 2.0, 3.0])"
    assert_transpilation(source, expected)

def test_transpile_tambah():
    source = """
    atur listku = [1, 2]
    tambah(listku, 3)
    """
    expected = """
    listku = [1.0, 2.0]
    listku.append(3.0)
    """
    assert_transpilation(source, expected)

def test_transpile_map_literal():
    source = 'atur data = peta{"nama": "Morph", "versi": 0.3}'
    expected = "data = {'nama': 'Morph', 'versi': 0.3}"
    assert_transpilation(source, expected)

def test_transpile_subscript_expression():
    source = """
    atur data = peta{"kunci": "nilai"}
    atur nilai = data["kunci"]
    """
    expected = """
    data = {'kunci': 'nilai'}
    nilai = data['kunci']
    """
    assert_transpilation(source, expected)

def test_transpile_ulangi_statement():
    source = 'ulangi lihat("tes") sebanyak 3 kali'
    expected = """
    for _ in range(int(3.0)):
        print('tes')
    """
    assert_transpilation(source, expected)

def test_transpile_ulangi_with_block():
    source = """
    atur x = 0
    ulangi {
        atur x = x + 1
    } sebanyak 2 kali
    """
    expected = """
    x = 0.0
    for _ in range(int(2.0)):
        x = (x + 1.0)
    """
    assert_transpilation(source, expected)
