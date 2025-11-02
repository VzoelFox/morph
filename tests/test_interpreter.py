# tests/test_interpreter.py
import pytest
from pathlib import Path
from tests.helpers import run_vzoel_code_capture_output

def test_hello_world_interpreter():
    # 1. Setup: Baca kode sumber
    source_path = Path(__file__).parent.parent / "examples" / "hello.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()

    # 2. Aksi: Jalankan kode dan tangkap output
    output = run_vzoel_code_capture_output(source_code)

    # 3. Verifikasi: Periksa apakah outputnya benar
    # Kita tambahkan \n karena print() menambahkan baris baru.
    assert output == ["Halo Dunia dari Vzoel Word v0.2!"]
