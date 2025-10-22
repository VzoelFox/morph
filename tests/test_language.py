import pytest
from tests.helpers import run_vzoel_code

def test_function_and_operators():
    source = """
    proses tambah(a, b) { kembali a + b }
    lihat(tambah(5, 3) * 2)
    """
    output = run_vzoel_code(source)
    assert output == "16.0\n"
