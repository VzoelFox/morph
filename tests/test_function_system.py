import pytest
from pathlib import Path
from tests.test_interpreter import run_vzoel_code

def test_user_defined_function():
    source_path = Path(__file__).parent.parent / "examples" / "fungsi.vz"
    with open(source_path, 'r', encoding='utf-8') as f:
        source_code = f.read()
    output = run_vzoel_code(source_code)
    expected_output = "15.5\n"
    assert output == expected_output
