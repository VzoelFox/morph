# morph.py
# Ini adalah titik masuk utama untuk menjalankan file Bahasa Morph (.fox).

import sys
from pathlib import Path
from interpreter.lexer import Lexer
from interpreter.parser import Parser
from interpreter.interpreter import Interpreter
from analyzer.static_analyzer import StaticAnalyzer
from transpiler.transpiler import Transpiler

# Buat instance komponen sekali saja untuk efisiensi
morph_interpreter = Interpreter()
morph_analyzer = StaticAnalyzer()
morph_transpiler = Transpiler()

def run_file(filepath: Path):
    """Membaca file dan menjalankannya melalui analyzer dan interpreter Morph."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source_code = f.read()
    except FileNotFoundError:
        print(f"Error: File '{filepath}' tidak ditemukan.")
        return

    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()

    if parser.errors:
        print("Proses berhenti karena ada error saat parsing:")
        for error in parser.errors:
            print(error)
        return

    # 1. Jalankan analisis statis
    analysis_passed = morph_analyzer.analyze(program)

    # 2. Jika analisis berhasil, jalankan interpreter
    if analysis_passed:
        morph_interpreter.interpret(program)
    else:
        print("\nProses berhenti karena ada error analisis statis.")

def transpile_file(filepath: Path):
    """Membaca file .fox dan mentranspilasinya menjadi file .py."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source_code = f.read()
    except FileNotFoundError:
        print(f"Error: File '{filepath}' tidak ditemukan.")
        return

    lexer = Lexer(source_code)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    program = parser.parse()

    if parser.errors:
        print("Transpilasi gagal karena ada error saat parsing:")
        for error in parser.errors:
            print(error)
        return

    # Jalankan transpiler
    python_code = morph_transpiler.transpile(program)

    # Tentukan nama file output
    output_filepath = filepath.with_suffix('.py')

    try:
        with open(output_filepath, 'w', encoding='utf-8') as f:
            f.write(python_code)
        print(f"Berhasil mentranspilasi '{filepath}' ke '{output_filepath}'")
    except IOError as e:
        print(f"Gagal menulis ke file '{output_filepath}': {e}")


def main():
    """Fungsi utama untuk menangani argumen baris perintah."""
    args = sys.argv[1:]
    if not args:
        print("Penggunaan: python morph.py [--transpile] <nama_file.fox>")
        sys.exit(1)

    if args[0] == '--transpile':
        if len(args) != 2:
            print("Penggunaan: python morph.py --transpile <nama_file.fox>")
            sys.exit(1)
        filepath = Path(args[1])
        transpile_file(filepath)
    else:
        if len(args) != 1:
            print("Penggunaan: python morph.py <nama_file.fox>")
            sys.exit(1)
        filepath = Path(args[0])
        run_file(filepath)

if __name__ == "__main__":
    main()
