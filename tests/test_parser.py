import pytest
from pathlib import Path
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
def test_hello_world_parser():
    # 1. Setup: Dapatkan token dari Lexer
    source_path = Path(__file__).parent.parent / "examples" / "hello.fox"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()

    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()

    # 2. Aksi: Parse token menjadi AST
    parser = Parser(tokens)
    program_ast = parser.parse()

    # 3. Verifikasi: Periksa struktur AST

    # a. Harus ada 2 pernyataan (atur dan lihat)
    assert isinstance(program_ast, ast.Program)
    assert len(program_ast.statements) == 2, "Diharapkan 2 pernyataan."

    # b. Verifikasi pernyataan pertama: atur pesan = "..."
    stmt1 = program_ast.statements[0]
    assert isinstance(stmt1, ast.AturStatement)
    assert stmt1.name.literal == "pesan"
    assert isinstance(stmt1.initializer, ast.Literal)
    assert stmt1.initializer.value == "Halo Dunia dari Vzoel Word v0.2!"

    # c. Verifikasi pernyataan kedua: lihat(pesan)
    stmt2 = program_ast.statements[1]
    assert isinstance(stmt2, ast.ExpressionStatement)

    # d. Ekspresi di dalamnya harus berupa pemanggilan fungsi
    expr = stmt2.expression
    assert isinstance(expr, ast.FunctionCall)

    # e. Fungsi yang dipanggil adalah 'lihat'
    assert isinstance(expr.callee, ast.Variable)
    assert expr.callee.name.literal == "lihat"

    # f. Argumennya adalah variabel 'pesan'
    assert len(expr.arguments) == 1
    arg1 = expr.arguments[0]
    assert isinstance(arg1, ast.Variable)
    assert arg1.name.literal == "pesan"

def parse_helper(source_code):
    """Helper untuk mem-parse kode dan mengembalikan pernyataan pertama."""
    tokens = Lexer(source_code).scan_tokens()
    program = Parser(tokens).parse()
    assert not Parser(tokens).errors
    assert len(program.statements) == 1
    return program.statements[0]

def test_parser_jika_statement():
    """Memastikan parser dapat meng-handle 'jika-maka'."""
    stmt = parse_helper("jika benar maka atur x = 1;")
    assert isinstance(stmt, ast.JikaStatement)
    assert isinstance(stmt.condition, ast.Literal)
    assert stmt.condition.value is True
    assert isinstance(stmt.then_branch, ast.AturStatement)
    assert stmt.else_branch is None

def test_parser_jika_lainnya_statement():
    """Memastikan parser dapat meng-handle 'jika-maka-lainnya'."""
    stmt = parse_helper("jika salah maka atur x = 1; lainnya atur y = 2;")
    assert isinstance(stmt, ast.JikaStatement)
    assert isinstance(stmt.condition, ast.Literal)
    assert stmt.condition.value is False
    assert isinstance(stmt.then_branch, ast.AturStatement)
    assert isinstance(stmt.else_branch, ast.AturStatement)
