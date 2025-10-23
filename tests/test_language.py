# tests/test_language.py
import pytest
from tests.helpers import run_vzoel_code_capture_output

def test_function_call():
    source = """
    proses tambah(a, b) { kembali a + b }
    lihat(tambah(5, 3))
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["8.0"]
