# tests/test_module_system.py
import pytest
from pathlib import Path
from tests.helpers import run_vzoel_code_capture_output

def test_module_fallback_and_property_access():
    source_path = Path(__file__).parent.parent / "examples" / "modul_utama.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    output = run_vzoel_code_capture_output(source_code)
    output_str = "".join(output)
    assert "Mencoba memuat modul yang tidak ada..." in output_str
    assert "Modul berhasil dimuat." in output_str
    assert "Tipe dari Cadangan" in output_str
