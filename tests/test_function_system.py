import pytest
from tests.helpers import run_vzoel_code_capture_output

def test_user_defined_function():
    source_code = """
    proses tambah(a, b) {
      kembali a + b
    }
    atur hasil = tambah(10, 5.5)
    lihat(hasil)
    """
    output = run_vzoel_code_capture_output(source_code)
    expected_output = ["15.5"]
    assert output == expected_output
