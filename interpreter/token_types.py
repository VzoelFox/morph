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
