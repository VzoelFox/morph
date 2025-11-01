import pytest
from tests.test_interpreter import run_vzoel_code

def test_user_defined_function():
    source_code = """
    proses tambah(a, b) {
      kembali a + b
    }
    atur hasil = tambah(10, 5.5)
    lihat(hasil)
    """
    output = run_vzoel_code(source_code)
    expected_output = "15.5\n"
    assert output == expected_output
