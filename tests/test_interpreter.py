import pytest
from pathlib import Path
import io, sys
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter
from interpreter.errors import VzoelRuntimeException

def run_vzoel_code(source_code: str) -> str:
    original_stdout = sys.stdout
    sys.stdout = captured_output = io.StringIO()
    try:
        lexer = Lexer(source_code)
        tokens = lexer.scan_tokens()
        parser = Parser(tokens)
        program_ast = parser.parse()
        # Jika ada error parsing, kita ingin tesnya gagal di sini
        assert not parser.errors, f"Parser errors: {parser.errors}"
        interpreter = Interpreter()
        interpreter.interpret(program_ast)
    finally:
        sys.stdout = original_stdout
    return captured_output.getvalue()

def test_hello_world_interpreter():
    source_path = Path(__file__).parent.parent / "examples" / "hello.fox"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    output = run_vzoel_code(source_code)
    expected_output = "Halo Dunia dari Vzoel Word v0.2!\n"
    assert output == expected_output

def run_for_runtime_error(source_code: str):
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    assert not lexer.errors
    parser = Parser(tokens)
    program = parser.parse()
    assert not parser.errors, f"Parser errors: {parser.errors}"
    interpreter = Interpreter()
    interpreter.interpret(program)

def test_interpreter_division_by_zero():
    source = "lihat(10 / 0);"
    with pytest.raises(VzoelRuntimeException) as e:
        run_for_runtime_error(source)
    assert "Tidak bisa membagi dengan nol" in e.value.message

def test_interpreter_invalid_operand_type_addition():
    source = 'lihat("halo" + 5);'
    with pytest.raises(VzoelRuntimeException) as e:
        run_for_runtime_error(source)
    assert "Operan harus dua angka atau dua string" in e.value.message

def test_interpreter_invalid_operand_type_subtraction():
    source = 'lihat("halo" - 5);'
    with pytest.raises(VzoelRuntimeException) as e:
        run_for_runtime_error(source)
    assert "Kedua operan harus berupa angka" in e.value.message

def test_interpreter_index_out_of_bounds():
    source = """
    atur daftar = [1, 2, 3];
    lihat(daftar[3]);
    """
    with pytest.raises(VzoelRuntimeException) as e:
        run_for_runtime_error(source)
    assert "Indeks 3 di luar batas" in e.value.message

def test_interpreter_key_not_in_map():
    source = """
    atur petaku = peta{"kunci": "nilai"};
    lihat(petaku["salah"]);
    """
    with pytest.raises(VzoelRuntimeException) as e:
        run_for_runtime_error(source)
    assert "Kunci 'salah' tidak ditemukan di peta" in e.value.message
