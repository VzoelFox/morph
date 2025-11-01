# tests/test_builtins.py
from tests.helpers import run_vzoel_code_capture_output
import pytest

def test_kekecil_success():
    source = 'lihat(kekecil("TEST"))'
    output = run_vzoel_code_capture_output(source)
    assert output == ["test"]

def test_kekecil_invalid_argument():
    source = 'kekecil(123)'
    output = run_vzoel_code_capture_output(source)
    assert "Error runtime: Argumen untuk 'kekecil' harus berupa string." in output[0]

def test_pangkat_success():
    source = "lihat(pangkat(2, 3))"
    output = run_vzoel_code_capture_output(source)
    assert output == ["8.0"]

def test_pangkat_invalid_arguments():
    source = 'pangkat("a", 2)'
    output = run_vzoel_code_capture_output(source)
    assert "Error runtime: Argumen untuk 'pangkat' harus berupa angka." in output[0]
