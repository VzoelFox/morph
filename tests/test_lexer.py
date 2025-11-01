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

from pathlib import Path

def test_hello_world_lexer():
    # Path ke file contoh
    source_path = Path(__file__).parent.parent / "examples" / "hello.vz"

    # Baca source code
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()

    # Inisialisasi lexer dan scan token
    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()

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

    # Bandingkan setiap token
    for i, (token_type, literal) in enumerate(expected_tokens):
        assert tokens[i].type == token_type, f"Token #{i} tipe tidak cocok. Diharapkan: {token_type}, Dihasilkan: {tokens[i].type}"
        assert tokens[i].literal == literal, f"Token #{i} literal tidak cocok. Diharapkan: '{literal}', Dihasilkan: '{tokens[i].literal}'"
