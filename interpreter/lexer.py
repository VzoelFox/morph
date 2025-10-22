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
            "atur": TokenType.ATUR, "jika": TokenType.JIKA, "maka": TokenType.MAKA,
            "tidak": TokenType.TIDAK, "berhasil": TokenType.BERHASIL, "proses": TokenType.PROSES,
            "ambil": TokenType.AMBIL, "dari": TokenType.DARI, "kembali": TokenType.KEMBALI,
            "benar": TokenType.BENAR, "salah": TokenType.SALAH
        }
    def scan_tokens(self) -> list[Token]:
        while not self._is_at_end():
            self.start = self.current
            self._scan_token()
        self.tokens.append(Token(TokenType.EOF, "", self.line))
        return self.tokens
    def _scan_token(self):
        char = self._advance()
        if char in [' ', '\r', '\t']: pass
        elif char == '\n': self.line += 1
        elif char == '(': self._add_token(TokenType.KURUNG_BUKA)
        elif char == ')': self._add_token(TokenType.KURUNG_TUTUP)
        elif char == '{': self._add_token(TokenType.KURAWAL_BUKA)
        elif char == '}': self._add_token(TokenType.KURAWAL_TUTUP)
        elif char == ',': self._add_token(TokenType.KOMA)
        elif char == '.': self._add_token(TokenType.TITIK)
        elif char == '+': self._add_token(TokenType.PLUS)
        elif char == '=': self._add_token(TokenType.SAMA_DENGAN)
        elif char == '#':
            while self._peek() != '\n' and not self._is_at_end(): self._advance()
        elif char == '"': self._string()
        elif self._is_digit(char): self._number()
        elif self._is_alpha(char): self._identifier()
        else: self._add_token(TokenType.TIDAK_DIKENALI)
    def _identifier(self):
        while self._is_alpha_numeric(self._peek()): self._advance()
        text = self.source[self.start:self.current]
        token_type = self.KEYWORDS.get(text, TokenType.IDENTIFIER)
        self._add_token(token_type)
    def _number(self):
        while self._is_digit(self._peek()): self._advance()
        if self._peek() == '.' and self._is_digit(self._peek_next()):
            self._advance()
            while self._is_digit(self._peek()): self._advance()
        self._add_token(TokenType.NUMBER, float(self.source[self.start:self.current]))
    def _string(self):
        while self._peek() != '"' and not self._is_at_end():
            if self._peek() == '\n': self.line += 1
            self._advance()
        if self._is_at_end(): return
        self._advance()
        value = self.source[self.start + 1:self.current - 1]
        self._add_token(TokenType.STRING, value)
    def _is_at_end(self): return self.current >= len(self.source)
    def _advance(self):
        char = self.source[self.current]
        self.current += 1
        return char
    def _add_token(self, type, literal=None):
        text = self.source[self.start:self.current]
        if literal is None: literal = text
        self.tokens.append(Token(type, literal, self.line))
    def _peek(self):
        if self._is_at_end(): return '\0'
        return self.source[self.current]
    def _peek_next(self):
        if self.current + 1 >= len(self.source): return '\0'
        return self.source[self.current + 1]
    def _is_digit(self, char): return '0' <= char <= '9'
    def _is_alpha(self, char): return ('a' <= char <= 'z') or ('A' <= char <= 'Z') or char == '_'
    def _is_alpha_numeric(self, char): return self._is_alpha(char) or self._is_digit(char)
