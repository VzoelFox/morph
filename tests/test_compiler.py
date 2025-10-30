"""
Pengujian untuk Compiler, memastikan bahwa AST dikompilasi dengan benar
menjadi Intermediate Representation (IR).
"""

import pytest

from interpreter.lexer import Lexer
from interpreter.parser import Parser
from compiler.compiler import Compiler
from compiler import instructions

def compile_code(source_code):
    """Sebuah helper untuk menjalankan pipeline: source -> lexer -> parser -> compiler."""
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()

    assert not parser.errors, f"Terdapat kesalahan parsing: {parser.errors}"

    compiler = Compiler()
    return compiler.compile(program)

def test_compile_number_literal():
    """Memastikan angka literal dikompilasi menjadi instruksi LoadConst."""
    ir = compile_code("5;")

    assert len(ir) == 1
    instruction = ir[0]

    assert isinstance(instruction, instructions.LoadConst)
    assert instruction.value == 5.0
    assert instruction.dest == "t1"

def test_compile_string_literal():
    """Memastikan string literal dikompilasi menjadi instruksi LoadConst."""
    ir = compile_code('"hello";')

    assert len(ir) == 1
    instruction = ir[0]

    assert isinstance(instruction, instructions.LoadConst)
    assert instruction.value == "hello"
    assert instruction.dest == "t1"

def test_compile_boolean_literals():
    """Memastikan boolean (benar/salah) dikompilasi menjadi LoadConst."""
    ir_true = compile_code("benar;")
    assert len(ir_true) == 1
    assert isinstance(ir_true[0], instructions.LoadConst)
    assert ir_true[0].value is True

    ir_false = compile_code("salah;")
    assert len(ir_false) == 1
    assert isinstance(ir_false[0], instructions.LoadConst)
    assert ir_false[0].value is False

def test_compile_unary_negation():
    """Memastikan ekspresi negasi unary dikompilasi dengan benar."""
    ir = compile_code("-10;")

    assert len(ir) == 2

    # Instruksi pertama: Muat konstanta 10
    assert isinstance(ir[0], instructions.LoadConst)
    assert ir[0].value == 10.0
    assert ir[0].dest == "t1"

    # Instruksi kedua: Negasikan nilai dari t1
    assert isinstance(ir[1], instructions.Negate)
    assert ir[1].operand == "t1"
    assert ir[1].dest == "t2"

def test_compile_binary_addition():
    """Memastikan ekspresi penjumlahan biner dikompilasi dengan benar."""
    ir = compile_code("5 + 10;")

    assert len(ir) == 3

    # Instruksi 1: Muat 5
    assert isinstance(ir[0], instructions.LoadConst)
    assert ir[0].value == 5.0
    assert ir[0].dest == "t1"

    # Instruksi 2: Muat 10
    assert isinstance(ir[1], instructions.LoadConst)
    assert ir[1].value == 10.0
    assert ir[1].dest == "t2"

    # Instruksi 3: Tambahkan t1 dan t2
    assert isinstance(ir[2], instructions.Add)
    assert ir[2].left == "t1"
    assert ir[2].right == "t2"
    assert ir[2].dest == "t3"

def test_compile_variable_declaration():
    """Memastikan deklarasi variabel dikompilasi menjadi LoadConst diikuti oleh StoreVar."""
    ir = compile_code("atur x = 10;")

    assert len(ir) == 2

    # Instruksi 1: Muat konstanta 10
    assert isinstance(ir[0], instructions.LoadConst)
    assert ir[0].value == 10.0
    assert ir[0].dest == "t1"

    # Instruksi 2: Simpan nilai dari t1 ke variabel 'x'
    assert isinstance(ir[1], instructions.StoreVar)
    assert ir[1].name == "x"
    assert ir[1].src == "t1"

def test_compile_variable_access():
    """Memastikan akses variabel dikompilasi menjadi instruksi LoadVar."""
    ir = compile_code("atur a = 5; a;")

    assert len(ir) == 3

    # Instruksi 1 & 2: Deklarasi variabel
    assert isinstance(ir[0], instructions.LoadConst)
    assert isinstance(ir[1], instructions.StoreVar)

    # Instruksi 3: Muat nilai dari 'a' ke temporary baru
    assert isinstance(ir[2], instructions.LoadVar)
    assert ir[2].name == "a"
    assert ir[2].dest == "t2"
