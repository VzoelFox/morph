# tests/test_function_system.py
import pytest
from pathlib import Path
from tests.helpers import run_vzoel_code_capture_output

def test_user_defined_function():
    source_path = Path(__file__).parent.parent / "examples" / "fungsi.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    output = run_vzoel_code_capture_output(source_code)
    assert output == ["Halo dari proses!", "Hasilnya adalah 8.0"]
