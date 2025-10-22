from .token_types import TokenType
from .token import Token

class Lexer:
    def __init__(self, source_code: str):
        self.source = source_code
        self.tokens = []
        self.start = 0
        self.current = 0
        self.line = 1
        self.KEYWORDS = {
            "atur": TokenType.ATUR, "proses": TokenType.PROSES, "kembali": TokenType.KEMBALI,
            "jika": TokenType.JIKA, "maka": TokenType.MAKA, "benar": TokenType.BENAR,
            "salah": TokenType.SALAH, "management": TokenType.MANAGEMENT, "tunggu": TokenType.TUNGGU,
            "dan": TokenType.DAN, "atau": TokenType.ATAU, "tidak": TokenType.TIDAK,
            "dari": TokenType.DARI, "ambil": TokenType.AMBIL, "berhasil": TokenType.BERHASIL,
            "lihat": TokenType.IDENTIFIER # Diperlakukan sebagai identifier
        }

    def scan_tokens(self) -> list[Token]:
        while not self._is_at_end():
            self.start = self.current
            self._scan_token()
        self.tokens.append(Token(TokenType.EOF, "", self.line))
        return self.tokens

    def _scan_token(self):
        char = self._advance()
        if char in ' \r\t': return
        if char == '\n': self.line += 1; return

        # Single-char tokens
        if char == '(': self._add_token(TokenType.KURUNG_BUKA); return
        if char == ')': self._add_token(TokenType.KURUNG_TUTUP); return
        if char == '{': self._add_token(TokenType.KURAWAL_BUKA); return
        if char == '}': self._add_token(TokenType.KURAWAL_TUTUP); return
        if char == ',': self._add_token(TokenType.KOMA); return
        if char == '.': self._add_token(TokenType.TITIK); return
        if char == '+': self._add_token(TokenType.PLUS); return
        if char == '-': self._add_token(TokenType.MINUS); return
        if char == '*': self._add_token(TokenType.BINTANG); return

        # One or two char tokens
        if char == '!': self._add_token(TokenType.TIDAK_SAMA_DENGAN if self._match('=') else TokenType.TIDAK_LOGIS); return
        if char == '=': self._add_token(TokenType.SAMA_DENGAN_SAMA_DENGAN if self._match('=') else TokenType.SAMA_DENGAN); return
        if char == '<': self._add_token(TokenType.KURANG_DARI_SAMA_DENGAN if self._match('=') else TokenType.KURANG_DARI); return
        if char == '>': self._add_token(TokenType.LEBIH_DARI_SAMA_DENGAN if self._match('=') else TokenType.LEBIH_DARI); return

        # Slash or comment
        if char == '/':
            if self._match('/'): # A comment goes until the end of the line.
                while self._peek() != '\n' and not self._is_at_end(): self._advance()
            else: self._add_token(TokenType.GARIS_MIRING)
            return

        # Literals
        if char == '"': self._string(); return
        if char.isdigit(): self._number(); return
        if char.isalpha() or char == '_': self._identifier(); return

        self._add_token(TokenType.TIDAK_DIKENALI)

    def _identifier(self):
        while self._peek().isalnum() or self._peek() == '_': self._advance()
        text = self.source[self.start:self.current]
        type = self.KEYWORDS.get(text, TokenType.IDENTIFIER)
        self._add_token(type)

    def _number(self):
        while self._peek().isdigit(): self._advance()
        is_float = False
        if self._peek() == '.' and self._peek_next().isdigit():
            is_float = True
            self._advance()
            while self._peek().isdigit(): self._advance()

        num_str = self.source[self.start:self.current]
        value = float(num_str) if is_float else int(num_str)
        self._add_token(TokenType.NUMBER, value)

    def _string(self):
        while self._peek() != '"' and not self._is_at_end():
            if self._peek() == '\n': self.line += 1
            self._advance()
        if self._is_at_end(): return # Unterminated string
        self._advance() # The closing ".
        value = self.source[self.start + 1:self.current - 1]
        self._add_token(TokenType.STRING, value)

    def _match(self, expected: str) -> bool:
        if self._is_at_end(): return False
        if self.source[self.current] != expected: return False
        self.current += 1
        return True

    def _peek(self):
        if self._is_at_end(): return '\0'
        return self.source[self.current]

    def _peek_next(self):
        if self.current + 1 >= len(self.source): return '\0'
        return self.source[self.current + 1]

    def _is_at_end(self): return self.current >= len(self.source)

    def _advance(self):
        self.current += 1
        return self.source[self.current - 1]

    def _add_token(self, type, literal=None):
        text = self.source[self.start:self.current]
        self.tokens.append(Token(type, literal if literal is not None else text, self.line))
