import pytest
from interpreter.lexer import Lexer
from interpreter.parser import Parser
import interpreter.ast_nodes as ast

def test_parser_basic():
    source = 'atur pesan = "halo"'
    tokens = Lexer(source).scan_tokens()
    program = Parser(tokens).parse()
    assert len(program.statements) == 1
    stmt = program.statements[0]
    assert isinstance(stmt, ast.AturStatement)
    assert stmt.name.literal == "pesan"
    assert isinstance(stmt.initializer, ast.Literal)
