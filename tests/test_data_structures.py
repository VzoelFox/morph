# tests/test_data_structures.py
import pytest
from tests.helpers import run_vzoel_code_capture_output

def test_list_literal_and_panjang():
    source = """
    atur data = [1, "dua", benar]
    lihat(panjang(data))
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["3"]

def test_list_tambah_element():
    source = """
    atur data = [1]
    tambah(data, 2)
    tambah(data, "tiga")
    lihat(panjang(data))
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["3"]

def test_nested_list():
    source = """
    atur data = [[1, 2], [3, 4]]
    lihat(panjang(data))
    """
    output = run_vzoel_code_capture_output(source)
    assert output == ["2"]
