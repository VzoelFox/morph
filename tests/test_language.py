import pytest
from tests.helpers import run_vzoel_code

def test_functions_and_operators():
    source = """
    proses fib(n) {
        jika n < 2 maka { kembali n }
        kembali fib(n - 2) + fib(n - 1)
    }
    lihat(fib(10))
    """
    output = run_vzoel_code(source)
    assert output == "55\n"

def test_scoping():
    source = """
    atur a = "global"
    {
        atur a = "lokal"
        lihat(a)
    }
    lihat(a)
    """
    output = run_vzoel_code(source)
    assert output == "lokal\nglobal\n"
def test_function_and_operators():
    source = """
    proses tambah(a, b) { kembali a + b }
    lihat(tambah(5, 3) * 2)
    """
    output = run_vzoel_code(source)
    assert output == "16.0\n"
