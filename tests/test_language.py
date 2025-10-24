import pytest
from tests.helpers import run_vzoel_code_capture_output

def test_function_and_operators():
    source = """
    proses tambah(a, b) { kembali a + b }
    lihat(tambah(5, 3))
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["8.0"]

def test_ulangi_loop():
    source = """
    ulangi lihat("halo") sebanyak 3 kali
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["halo", "halo", "halo"]
