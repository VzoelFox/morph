from enum import Enum, auto

class TokenType(Enum):
    # Keywords
    ATUR = auto(); PROSES = auto(); KEMBALI = auto(); JIKA = auto(); MAKA = auto()
    BENAR = auto(); SALAH = auto(); MANAGEMENT = auto(); TUNGGU = auto()
    DAN = auto(); ATAU = auto(); TIDAK = auto(); DARI = auto(); AMBIL = auto()
    BERHASIL = auto()

    # Symbols & Operators
    SAMA_DENGAN = auto(); PLUS = auto(); MINUS = auto(); BINTANG = auto(); GARIS_MIRING = auto()
    TITIK = auto(); KOMA = auto(); KURUNG_BUKA = auto(); KURUNG_TUTUP = auto()
    KURAWAL_BUKA = auto(); KURAWAL_TUTUP = auto()
    TIDAK_LOGIS = auto()

    # Comparison
    SAMA_DENGAN_SAMA_DENGAN = auto(); TIDAK_SAMA_DENGAN = auto()
    LEBIH_DARI = auto(); KURANG_DARI = auto()
    LEBIH_DARI_SAMA_DENGAN = auto(); KURANG_DARI_SAMA_DENGAN = auto()

    # Literals & Identifier
    IDENTIFIER = auto(); STRING = auto(); NUMBER = auto()

    # Misc
    EOF = auto(); TIDAK_DIKENALI = auto()
# interpreter/token_types.py
from enum import Enum
class TokenType(Enum):
    ATUR="atur"; JIKA="jika"; MAKA="maka"; PROSES="proses"; KEMBALI="kembali"; BENAR="benar"; SALAH="salah"
    SAMA_DENGAN="="; PLUS="+"; MINUS="-"; BINTANG="*"; GARIS_MIRING="/"; SAMA_DENGAN_SAMA_DENGAN="=="; TIDAK_SAMA_DENGAN="!="; LEBIH_DARI=">"; KURANG_DARI="<"
    KURUNG_BUKA="("; KURUNG_TUTUP=")"; KURAWAL_BUKA="{"; KURAWAL_TUTUP="}"
    IDENTIFIER="ID"; STRING="STR"; NUMBER="NUM"; EOF="eof"
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
    SAMA_DENGAN = "="
    PLUS = "+"
    TITIK = "."
    KOMA = ","
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
    TITIK = "."
    KURUNG_BUKA = "("
    KURUNG_TUTUP = ")"
    KURAWAL_BUKA = "{"
    KURAWAL_TUTUP = "}"
    IDENTIFIER = "IDENTIFIER"
    STRING = "STRING"
    NUMBER = "NUMBER"
    TIDAK_DIKENALI = "TIDAK_DIKENALI"
    EOF = "EOF"

    # Literals & Identifier
    IDENTIFIER = "IDENTIFIER" # Nama variabel atau proses
    STRING = "STRING"
    NUMBER = "NUMBER"

    # Lain-lain
    TIDAK_DIKENALI = "TIDAK_DIKENALI"
    EOF = "EOF" # End of File
