import pytest
from pathlib import Path
from tests.helpers import run_vzoel_code_capture_output

def test_module_fallback_and_property_access():
    source_code = """
    jika atur App = ambil dari "non_existent.fox" tidak berhasil maka
        atur App = ambil dari "examples/modul_prod.fox";

    lihat(App.tipe);
    """

    output = run_vzoel_code_capture_output(source_code)

    assert "Produksi" in "".join(output)
