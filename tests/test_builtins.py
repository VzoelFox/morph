# tests/test_builtins.py
from tests.helpers import run_vzoel_code_capture_output
import pytest

def test_akar_success():
    source = "lihat(akar(16))"
    output = run_vzoel_code_capture_output(source)
    assert output == ["4.0"]

def test_akar_invalid_argument():
    source = 'akar("abc")'
    output = run_vzoel_code_capture_output(source)
    assert "Error runtime: Argumen untuk 'akar' harus berupa angka." in output[0]

def test_pangkat_success():
    source = "lihat(pangkat(2, 3))"
    output = run_vzoel_code_capture_output(source)
    assert output == ["8.0"]

def test_pangkat_invalid_arguments():
    source = 'pangkat("a", 2)'
    output = run_vzoel_code_capture_output(source)
    assert "Error runtime: Argumen untuk 'pangkat' harus berupa angka." in output[0]
