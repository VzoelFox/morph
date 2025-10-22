# interpreter/token_types.py
from enum import Enum
class TokenType(Enum):
    ATUR="atur"; JIKA="jika"; MAKA="maka"; PROSES="proses"; KEMBALI="kembali"; BENAR="benar"; SALAH="salah"
    SAMA_DENGAN="="; PLUS="+"; MINUS="-"; BINTANG="*"; GARIS_MIRING="/"; SAMA_DENGAN_SAMA_DENGAN="=="; TIDAK_SAMA_DENGAN="!="; LEBIH_DARI=">"; KURANG_DARI="<"
    KURUNG_BUKA="("; KURUNG_TUTUP=")"; KURAWAL_BUKA="{"; KURAWAL_TUTUP="}"
    IDENTIFIER="ID"; STRING="STR"; NUMBER="NUM"; EOF="eof"
