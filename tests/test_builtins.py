# tests/test_builtins.py
from tests.helpers import run_vzoel_code_capture_output
import pytest
from interpreter.errors import VzoelRuntimeException

def test_ke_kecil_success():
    source = 'lihat(ke_kecil("TEST"))'
    output = run_vzoel_code_capture_output(source)
    assert output == ["test"]

from interpreter.errors import VzoelRuntimeException

def test_ke_kecil_invalid_argument():
    source = 'ke_kecil(123)'
    with pytest.raises(VzoelRuntimeException) as e:
        run_vzoel_code_capture_output(source)
    assert "Argumen untuk 'ke_kecil' harus berupa string." in str(e.value)

def test_akar_success():
    source = "lihat(akar(16))"
    output = run_vzoel_code_capture_output(source)
    assert output == ["4.0"]

def test_akar_invalid_argument():
    source = 'akar("abc")'
    with pytest.raises(VzoelRuntimeException) as e:
        run_vzoel_code_capture_output(source)
    assert "Argumen untuk 'akar' harus berupa angka." in str(e.value)

def test_pangkat_success():
    source = "lihat(pangkat(2, 3))"
    output = run_vzoel_code_capture_output(source)
    assert output == ["8.0"]

def test_pangkat_invalid_arguments():
    source = 'pangkat("a", 2)'
    with pytest.raises(VzoelRuntimeException) as e:
        run_vzoel_code_capture_output(source)
    assert "Argumen untuk 'pangkat' harus berupa angka." in str(e.value)
