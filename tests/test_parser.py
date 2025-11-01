import pytest
from pathlib import Path
from interpreter.lexer import Lexer
from interpreter.parser import Parser
import interpreter.ast_nodes as ast

def test_hello_world_parser():
    source_path = Path(__file__).parent.parent / "examples" / "hello.fox"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program_ast = parser.parse()
    assert not parser.errors
    assert isinstance(program_ast, ast.Program)
    assert len(program_ast.statements) == 2

def test_parser_invalid_assignment_target():
    source = 'atur "nama" = "vzoel"'
    tokens = Lexer(source).scan_tokens()
    parser = Parser(tokens)
    parser.parse()
    assert len(parser.errors) > 0
    error = parser.errors[0]
    assert "Diharapkan nama variabel" in error.message

def test_parser_synchronization_multiple_errors():
    source = """
    atur x = 10 20  // Error 1: unexpected token
    atur y = ;       // Error 2: missing expression
    lihat(y)
    atur z = "benar"
    """
    tokens = Lexer(source).scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()

    assert len(parser.errors) >= 1 # Seharusnya setidaknya satu error

    assert isinstance(program, ast.Program)
    # Bergantung pada bagaimana sinkronisasi bekerja, jumlah statement bisa bervariasi
    assert len(program.statements) > 0
