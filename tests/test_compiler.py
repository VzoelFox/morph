import pytest

from interpreter.lexer import Lexer
from interpreter.parser import Parser
from compiler.compiler import Compiler
from compiler.instructions import LoadConst

def _compile_source(source: str):
    """Helper untuk mengompilasi kode sumber Morph menjadi instruksi IR."""
    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse() # Mengubah pemanggilan ke parse()
    assert not parser.errors, "Terdapat error parsing"

    compiler = Compiler()
    instructions = compiler.compile(program)
    return instructions

def test_compile_literal_integer():
    """Memastikan literal integer dikompilasi menjadi LoadConst."""
    source = "123;"
    instructions = _compile_source(source)

    assert len(instructions) == 1
    instr = instructions[0]

    assert isinstance(instr, LoadConst)
    assert instr.value == 123.0
    assert instr.dest == "t1"

def test_compile_literal_string():
    """Memastikan literal string dikompilasi menjadi LoadConst."""
    source = '"hello";'
    instructions = _compile_source(source)

    assert len(instructions) == 1
    instr = instructions[0]

    assert isinstance(instr, LoadConst)
    assert instr.value == "hello"
    assert instr.dest == "t1"

def test_compile_multiple_literals():
    """Memastikan temporary dest diinkrementasi dengan benar."""
    source = "1; 2;"
    instructions = _compile_source(source)

    assert len(instructions) == 2

    instr1 = instructions[0]
    assert isinstance(instr1, LoadConst)
    assert instr1.value == 1.0
    assert instr1.dest == "t1"

    instr2 = instructions[1]
    assert isinstance(instr2, LoadConst)
    assert instr2.value == 2.0
    assert instr2.dest == "t2"
