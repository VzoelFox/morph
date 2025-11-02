# tests/test_core_features.py
from tests.helpers import run_vzoel_code_capture_output, run_vzoel_code
from interpreter.errors import VzoelRuntimeException
import pytest

# Parameterisasi untuk tes aritmatika
@pytest.mark.parametrize("expression, expected", [
    ("5 + 3", "8.0"),
    ("10 - 4", "6.0"),
    ("6 * 7", "42.0"),
    ("20 / 5", "4.0"),
    ("5 + 8 * 2", "21.0"),
    ("(5 + 8) * 2", "26.0"),
])
def test_arithmetic_operations(expression, expected):
    source = f"lihat({expression})"
    output = run_vzoel_code_capture_output(source)
    assert output == [expected]

def test_variable_assignment_and_print():
    source = """
    atur a = 10
    atur b = "halo"
    lihat(a)
    lihat(b)
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["10.0", "halo"]

def test_simple_function_call():
    source = """
    proses sapa(nama) {
        kembali "Halo, " + nama
    }
    lihat(sapa("Dunia"))
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["Halo, Dunia"]

def test_undefined_variable_raises_runtime_error():
    source = "lihat(x)"
    with pytest.raises(VzoelRuntimeException, match="Variabel 'x' tidak terdefinisi."):
        run_vzoel_code(source)

def test_ulangi_loop():
    source = """
    ulangi {
        lihat("halo")
    } sebanyak 2 kali
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["halo", "halo"]
