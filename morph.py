# morph.py
# Ini adalah titik masuk utama untuk menjalankan file Bahasa Morph (.fox).

import sys
from pathlib import Path
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter
from interpreter.errors import VzoelRuntimeException

# Buat interpreter sekali saja untuk efisiensi
morph_interpreter = Interpreter()

def run_file(filepath: Path):
    """Membaca file dan menjalankannya melalui interpreter Morph."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source_code = f.read()

        lexer = Lexer(source_code)
        tokens = lexer.scan_tokens()
        parser = Parser(tokens)
        program = parser.parse()

        # Jalankan program
        morph_interpreter.interpret(program)

    except FileNotFoundError:
        print(f"Error: File '{filepath}' tidak ditemukan.")
    except Exception as e:
        # Menangkap error tak terduga lainnya dari interpreter atau parser
        print(f"Terjadi error: {e}")

def main():
    """Fungsi utama untuk menangani argumen baris perintah."""
    if len(sys.argv) > 2:
        print("Penggunaan: python morph.py [nama_file.fox]")
    elif len(sys.argv) == 2:
        filepath = Path(sys.argv[1])
        run_file(filepath)
    else:
        print("Penggunaan: python morph.py [nama_file.fox]")

if __name__ == "__main__":
    main()
