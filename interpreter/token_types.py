from enum import Enum

class TokenType(Enum):
    # === struktur / deklarasi / import ===
    AMBIL = "AMBIL"
    DARI = "DARI"
    ATUR = "ATUR"
    PROSES = "PROSES"
    PETA = "PETA"

    # === kontrol alur ===
    JIKA = "JIKA"
    MAKA = "MAKA"
    LAINNYA = "LAINNYA"
    TIDAK = "TIDAK"
    TIDAK_AKAN = "TIDAK_AKAN"
    JANGAN = "JANGAN"
    BERHASIL = "BERHASIL"

    # === loop ===
    ULANGI = "ULANGI"
    SEBANYAK = "SEBANYAK"
    KALI = "KALI"
    TERUSKAN = "TERUSKAN"

    # === eksekusi / async style ===
    TUNGGU = "TUNGGU"
    LALU = "LALU"
    SAMBIL = "SAMBIL"

    # === lifecycle manajemen task ===
    MATIKAN = "MATIKAN"
    HENTIKAN = "HENTIKAN"

    # === hasil fungsi ===
    KEMBALI = "KEMBALI"

    # === literal / boolean ===
    BENAR = "BENAR"
    SALAH = "SALAH"

    # === akhir file ===
    ADS = "AKHIR_DARI_SEGALANYA"
    EOF = "EOF"

    # Token yang sudah ada dan dipertahankan
    PEMICU = "PEMICU"
    TERJADI = "TERJADI"
    HARUS = "HARUS"

    # simbol, operator, literal juga tetap:
    SAMA_DENGAN = "="
    PLUS = "+"
    MINUS = "-"
    BINTANG = "*"
    GARIS_MIRING = "/"
    SAMA_DENGAN_SAMA_DENGAN = "=="
    TIDAK_SAMA_DENGAN = "!="
    LEBIH_DARI = ">"
    KURANG_DARI = "<"
    LEBIH_DARI_SAMA_DENGAN = ">="
    KURANG_DARI_SAMA_DENGAN = "<="
    TITIK = "."
    KOMA = ","
    KURUNG_BUKA = "("
    KURUNG_TUTUP = ")"
    KURAWAL_BUKA = "{"
    KURAWAL_TUTUP = "}"
    KURUNG_SIKU_BUKA = "["
    KURUNG_SIKU_TUTUP = "]"
    TITIK_DUA = ":"

    IDENTIFIER = "IDENTIFIER"
    STRING = "STRING"
    NUMBER = "NUMBER"

    TIDAK_DIKENALI = "TIDAK_DIKENALI"
