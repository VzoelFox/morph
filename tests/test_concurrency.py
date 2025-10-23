# tests/test_concurrency.py
import pytest
from tests.helpers import run_vzoel_code_capture_output
import time

def test_management_basic_scheduling():
    source = """
    management {
        proses tugas(nama, jeda) {
            lihat("Mulai " + nama)
            tunggu jeda
            lihat("Selesai " + nama)
        }

        jalankan tugas("A", 0.02)
        jalankan tugas("B", 0.01)
    }
    """
    start_time = time.time()
    output = run_vzoel_code_capture_output(source)
    end_time = time.time()

    duration = end_time - start_time
    assert duration >= 0.02

    expected_output = [
        "Mulai A",
        "Mulai B",
        "Selesai B",
        "Selesai A",
    ]
    assert output == expected_output, f"Output aktual: {output}"

def test_sequential_execution_outside_management():
    source = """
    proses tugas(nama, jeda) {
        lihat("Mulai " + nama)
        tunggu jeda # 'tunggu' di luar 'management' seharusnya tidak berpengaruh
        lihat("Selesai " + nama)
    }

    tugas("A", 0.02)
    tugas("B", 0.01)
    """
    output = run_vzoel_code_capture_output(source)

    expected_output = [
        "Mulai A",
        "Selesai A",
        "Mulai B",
        "Selesai B",
    ]
    assert output == expected_output, f"Output aktual: {output}"
