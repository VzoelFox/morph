from enum import Enum

class TokenType(Enum):
    # Kata Kunci (Keywords)
    ATUR = "ATUR"
    LIHAT = "LIHAT"
    JIKA = "JIKA"
    MAKA = "MAKA"
    TIDAK = "TIDAK"
    BERHASIL = "BERHASIL"
    PROSES = "PROSES"
    KEMBALI = "KEMBALI"
    JALANKAN = "JALANKAN"
    TUNGGU = "TUNGGU"
    PEMICU = "PEMICU"
    TERJADI = "TERJADI"
    HARUS = "HARUS"
    BENAR = "BENAR"
    SALAH = "SALAH"

    # Simbol & Operator
    SAMA_DENGAN = "="
    PLUS = "+"
    KURUNG_BUKA = "("
    KURUNG_TUTUP = ")"
    KURAWAL_BUKA = "{"
    KURAWAL_TUTUP = "}"

    # Literals & Identifier
    IDENTIFIER = "IDENTIFIER" # Nama variabel atau proses
    STRING = "STRING"
    NUMBER = "NUMBER"

    # Lain-lain
    TIDAK_DIKENALI = "TIDAK_DIKENALI"
    EOF = "EOF" # End of File
