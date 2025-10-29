# morph.py
# Ini adalah titik masuk utama untuk menjalankan file Bahasa Morph (.fox).

import sys
from pathlib import Path
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter
from analyzer.static_analyzer import StaticAnalyzer
from interpreter.errors import VzoelRuntimeException

# Buat interpreter dan analyzer sekali saja untuk efisiensi
morph_interpreter = Interpreter()
morph_analyzer = StaticAnalyzer()

def run_file(filepath: Path):
    """Membaca file dan menjalankannya melalui analyzer dan interpreter Morph."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source_code = f.read()

        lexer = Lexer(source_code)
        tokens = lexer.scan_tokens()
        parser = Parser(tokens)
        program = parser.parse()

        # Jika ada error parsing, program akan menjadi None
        if not program:
            print("Proses berhenti karena ada error saat parsing.")
            return

        # 1. Jalankan analisis statis
        analysis_passed = morph_analyzer.analyze(program)

        # 2. Jika analisis berhasil, jalankan interpreter
        if analysis_passed:
            morph_interpreter.interpret(program)
        else:
            print("Proses berhenti karena ada error analisis statis.")


    except FileNotFoundError:
        print(f"Error: File '{filepath}' tidak ditemukan.")
    except Exception as e:
        # Menangkap error tak terduga lainnya
        print(f"Terjadi error tak terduga: {e}")

def main():
    """Fungsi utama untuk menangani argumen baris perintah."""
    if len(sys.argv) != 2:
        print("Penggunaan: python morph.py <nama_file.fox>")
        sys.exit(1)

    filepath = Path(sys.argv[1])
    run_file(filepath)

if __name__ == "__main__":
    main()
