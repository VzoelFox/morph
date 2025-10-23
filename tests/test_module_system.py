import pytest
from pathlib import Path
from tests.helpers import run_vzoel_code_capture_output

def test_module_fallback_and_property_access():
    # 1. Setup: Baca kode sumber dari program utama
    source_path = Path(__file__).parent.parent / "examples" / "modul_utama.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()

    # 2. Aksi: Jalankan kode dan tangkap output
    output = run_vzoel_code_capture_output(source_code)

    # 3. Verifikasi: Periksa apakah outputnya benar
    output_str = "".join(output)
    assert "Mencoba memuat modul yang tidak ada..." in output_str
    assert "Modul berhasil dimuat." in output_str
    assert "Produksi" in output_str
