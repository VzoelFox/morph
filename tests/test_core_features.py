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

def test_ulangi_loop_zero_times():
    """Test that loop with 0 iterations doesn't execute body"""
    source = """
    atur counter = 0
    ulangi {
        atur counter = counter + 1
    } sebanyak 0 kali
    lihat(counter)
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["0.0"]

def test_ulangi_loop_one_time():
    """Test loop with single iteration"""
    source = """
    ulangi {
        lihat("once")
    } sebanyak 1 kali
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["once"]

def test_ulangi_loop_nested():
    """Test nested loops"""
    source = """
    ulangi {
        ulangi {
            lihat("nested")
        } sebanyak 2 kali
    } sebanyak 2 kali
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["nested", "nested", "nested", "nested"]

def test_ulangi_loop_with_variable_count():
    """Test loop with variable as count"""
    source = """
    atur jumlah = 3
    ulangi {
        lihat("var")
    } sebanyak jumlah kali
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["var", "var", "var"]

def test_ulangi_loop_invalid_count_raises_error():
    """Test that non-numeric count raises error"""
    source = """
    ulangi {
        lihat("test")
    } sebanyak "bukan angka" kali
    """
    with pytest.raises(Exception) as exc_info:
        run_vzoel_code(source)
    assert "angka" in str(exc_info.value).lower()
