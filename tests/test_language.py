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

def test_management_system_success():
    source = """
    management sistem_sukses {
        bagian pertama {
            pecahan coba {
                lihat("Sukses")
            }
        }
    }
    jalankan sistem_sukses
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["Sukses"]

from interpreter.errors import VzoelRuntimeException

def test_management_system_fallback():
    source = """
    management sistem_fallback {
        bagian data_fetcher {
            pecahan dari_api {
                panggil_fungsi_yang_tidak_ada()
            }
            pecahan dari_cache {
                lihat("Menggunakan cache")
            }
        }
    }
    jalankan sistem_fallback
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["Menggunakan cache"]

def test_management_system_total_failure():
    source = """
    management sistem_gagal {
        bagian data_fetcher {
            pecahan dari_api {
                panggil_fungsi_yang_tidak_ada_1()
            }
            pecahan dari_cache {
                panggil_fungsi_yang_tidak_ada_2()
            }
        }
    }
    jalankan sistem_gagal
    """
    with pytest.raises(VzoelRuntimeException) as e:
        run_vzoel_code_capture_output(source)
    assert "Semua pecahan di bagian 'data_fetcher' gagal." in str(e.value)
