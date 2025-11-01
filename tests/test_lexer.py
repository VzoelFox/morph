import pytest
from interpreter.lexer import Lexer
from interpreter.token_types import TokenType
from pathlib import Path

def test_hello_world_lexer():
    source_path = Path(__file__).parent.parent / "examples" / "hello.fox"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()

    expected_token_types = [
        TokenType.ATUR, TokenType.IDENTIFIER, TokenType.SAMA_DENGAN, TokenType.STRING,
        TokenType.IDENTIFIER, TokenType.KURUNG_BUKA, TokenType.IDENTIFIER, TokenType.KURUNG_TUTUP,
            TokenType.ADS
    ]

    token_types = [t.type for t in tokens]
    assert token_types == expected_token_types

def test_lexer_illegal_character():
    source = "atur x = 10 & 20"
    lexer = Lexer(source)
    lexer.scan_tokens()
    assert len(lexer.errors) > 0
    assert "Karakter tidak dikenali: '&'" in lexer.errors[0].message

def test_lexer_unterminated_string():
    source = 'atur pesan = "halo dunia'
    lexer = Lexer(source)
    lexer.scan_tokens()
    assert len(lexer.errors) > 0
    assert "String tidak ditutup" in lexer.errors[0].message

def test_lexer_escape_sequences():
    source = r'"baris1\nbaris2\tindent"'
    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    assert not lexer.errors
    assert tokens[0].type == TokenType.STRING
    assert tokens[0].literal == "baris1\nbaris2\tindent"

    # Hapus token whitespace jika ada (desain lexer saat ini mengabaikannya, jadi ini hanya untuk keamanan)
    tokens = [t for t in tokens if t.type != TokenType.TIDAK_DIKENALI]

    # Tentukan urutan token yang diharapkan
    expected_tokens = [
        (TokenType.ATUR, "atur"),
        (TokenType.IDENTIFIER, "pesan"),
        (TokenType.SAMA_DENGAN, "="),
        (TokenType.STRING, "Halo Dunia dari Vzoel Word v0.2!"),
        (TokenType.IDENTIFIER, "lihat"),
        (TokenType.KURUNG_BUKA, "("),
        (TokenType.IDENTIFIER, "pesan"),
        (TokenType.KURUNG_TUTUP, ")"),
        (TokenType.ADS, ""),
    ]

    # Pastikan jumlah token sama
    assert len(tokens) == len(expected_tokens), f"Jumlah token tidak sesuai. Diharapkan: {len(expected_tokens)}, Dihasilkan: {len(tokens)}"

def test_lexer_multiline_string_unterminated():
    source = '"""halo'
    lexer = Lexer(source)
    lexer.scan_tokens()
    assert len(lexer.errors) > 0
    assert "String multi-baris tidak ditutup" in lexer.errors[0].message
