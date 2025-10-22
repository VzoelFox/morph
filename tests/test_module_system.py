import pytest
from pathlib import Path
from tests.test_interpreter import run_vzoel_code

def test_module_fallback():
    source_path = Path(__file__).parent.parent / "examples" / "modul_utama.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    output = run_vzoel_code(source_code)
    expected_output = "Produksi\n"
from tests.test_interpreter import run_vzoel_code # Gunakan helper dari tes interpreter

def test_module_fallback_and_property_access():
    # 1. Setup: Baca kode sumber dari program utama
    source_path = Path(__file__).parent.parent / "examples" / "modul_utama.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()

    # 2. Aksi: Jalankan kode dan tangkap output
    output = run_vzoel_code(source_code)

    # 3. Verifikasi: Periksa apakah outputnya benar
    expected_lines = [
        "Mencoba memuat modul yang tidak ada...",
        "Modul berhasil dimuat.",
        "Produksi"
    ]
    expected_output = "\n".join(expected_lines) + "\n"

    assert output == expected_output
