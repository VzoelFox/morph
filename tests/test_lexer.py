import pytest
from interpreter.lexer import Lexer
from interpreter.token_types import TokenType

def test_lexer_basic():
    source = 'atur pesan = "halo"\nlihat(pesan)'
    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    expected = [
        TokenType.ATUR, TokenType.IDENTIFIER, TokenType.SAMA_DENGAN, TokenType.STRING,
        TokenType.IDENTIFIER, TokenType.KURUNG_BUKA, TokenType.IDENTIFIER, TokenType.KURUNG_TUTUP,
        TokenType.ADS
    ]
    token_types = [t.type for t in tokens]
    assert token_types == expected
    assert not lexer.errors

def test_lexer_illegal_character():
    source = 'atur a = 10; @'
    lexer = Lexer(source)
    lexer.scan_tokens()
    assert len(lexer.errors) == 1
    assert "Karakter tidak dikenali: @" in lexer.errors[0]
