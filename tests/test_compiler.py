# tests/test_compiler.py
import pytest
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from compiler.compiler import Compiler
from compiler import instructions

def compile_source(source: str):
    """Helper untuk mengompilasi kode sumber menjadi instruksi."""
    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    ast = parser.parse()
    compiler = Compiler()
    return compiler.compile(ast)

def test_compile_grouping_expression():
    source = "(5 + 3) * 2"
    expected_instructions = [
        instructions.LoadConst(value=5, dest='t1'),
        instructions.LoadConst(value=3, dest='t2'),
        instructions.Add(left='t1', right='t2', dest='t3'),
        instructions.LoadConst(value=2, dest='t4'),
        instructions.Mul(left='t3', right='t4', dest='t5'),
    ]

    actual_instructions = compile_source(source)

    # Membandingkan instruksi tanpa memperhatikan nama temporary
    assert len(actual_instructions) == len(expected_instructions)
    for i, expected in enumerate(expected_instructions):
        actual = actual_instructions[i]
        assert isinstance(actual, type(expected))
        if isinstance(expected, instructions.LoadConst):
            assert actual.value == expected.value

def test_compile_variable():
    """Test compiling variable reference"""
    source = """
    atur x = 5
    lihat(x)
    """
    instructions = compile_source(source)

    # Should have LoadVar instruction for x
    var_instructions = [i for i in instructions if i.__class__.__name__ == 'LoadVar']
    assert len(var_instructions) > 0
    assert var_instructions[0].name == 'x'

def test_compile_grouping():
    """Test compiling grouped expression"""
    source = """
    atur result = (5 + 3) * 2
    """
    instructions = compile_source(source)

    # Should compile without errors
    assert len(instructions) > 0

def test_compile_variable_assignment_and_load():
    source = """
    atur a = 10
    atur b = a
    """
    expected_instructions = [
        instructions.LoadConst(value=10, dest='t1'),
        instructions.StoreVar(name='a', src='t1'),
        instructions.LoadVar(name='a', dest='t2'),
        instructions.StoreVar(name='b', src='t2'),
    ]

    actual_instructions = compile_source(source)

    assert len(actual_instructions) == len(expected_instructions)
    for i, expected in enumerate(expected_instructions):
        actual = actual_instructions[i]
        assert isinstance(actual, type(expected))
        if isinstance(expected, instructions.LoadConst):
            assert actual.value == expected.value
        elif isinstance(expected, instructions.StoreVar):
            assert actual.name == expected.name
        elif isinstance(expected, instructions.LoadVar):
            assert actual.name == expected.name
        elif isinstance(expected, (instructions.Add, instructions.Mul)):
            # Perbandingan ini mengasumsikan urutan temporary yang dapat diprediksi
            pass

def test_compile_simple_arithmetic():
    source = "10 - 4"
    expected_instructions = [
        instructions.LoadConst(value=10, dest='t1'),
        instructions.LoadConst(value=4, dest='t2'),
        instructions.Sub(left='t1', right='t2', dest='t3'),
    ]

    actual_instructions = compile_source(source)

    assert len(actual_instructions) == len(expected_instructions)
    for i, expected in enumerate(expected_instructions):
        actual = actual_instructions[i]
        assert isinstance(actual, type(expected))
        if isinstance(expected, instructions.LoadConst):
            assert actual.value == expected.value
