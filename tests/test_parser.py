import pytest
from interpreter.lexer import Lexer
from interpreter.parser import Parser
import interpreter.ast_nodes as ast

def test_parser_valid_statement():
    source = 'atur pesan = "halo";'
    tokens = Lexer(source).scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()
    assert not parser.errors
    assert len(program.statements) == 1

def test_parser_invalid_statement():
    source = 'atur x = ;'
    tokens = Lexer(source).scan_tokens()
    parser = Parser(tokens)
    parser.parse()
    assert len(parser.errors) > 0
    assert "Diharapkan sebuah ekspresi" in parser.errors[0].message
