# interpreter/token_types.py
from enum import Enum

class TokenType(Enum):
    # Kata Kunci (Keywords)
    ATUR = "ATUR"
    JIKA = "JIKA"
    MAKA = "MAKA"
    TIDAK = "TIDAK"
    BERHASIL = "BERHASIL"
    PROSES = "PROSES"
    AMBIL = "AMBIL"
    DARI = "DARI"
    KEMBALI = "KEMBALI"
    BENAR = "BENAR"
    SALAH = "SALAH"
    JALANKAN = "JALANKAN"
    TUNGGU = "TUNGGU"
    PEMICU = "PEMICU"
    TERJADI = "TERJADI"
    HARUS = "HARUS"
    PETA = "PETA"
    ULANGI = "ULANGI"
    SEBANYAK = "SEBANYAK"
    KALI = "KALI"

    # Simbol & Operator
    SAMA_DENGAN = "="
    PLUS = "+"
    MINUS = "-"
    BINTANG = "*"
    GARIS_MIRING = "/"
    SAMA_DENGAN_SAMA_DENGAN = "=="
    TIDAK_SAMA_DENGAN = "!="
    LEBIH_DARI = ">"
    KURANG_DARI = "<"
    TITIK = "."
    KOMA = ","
    TITIK_KOMA = ";"
    KURUNG_BUKA = "("
    KURUNG_TUTUP = ")"
    KURAWAL_BUKA = "{"
    KURAWAL_TUTUP = "}"
    KURUNG_SIKU_BUKA = "["
    KURUNG_SIKU_TUTUP = "]"
    TITIK_DUA = ":"

    # Literals & Identifier
    IDENTIFIER = "IDENTIFIER"
    STRING = "STRING"
    NUMBER = "NUMBER"

    # Lain-lain
    TIDAK_DIKENALI = "TIDAK_DIKENALI"
    ADS = "ADS" # Akhir Dari Segalanya
