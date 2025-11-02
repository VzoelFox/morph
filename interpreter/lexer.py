from .token_types import TokenType
from .token import Token
from .errors import LexerError

class Lexer:
    def __init__(self, source_code: str):
        self.source = source_code
        self.tokens = []
        self.errors = []
        self.start = 0
        self.current = 0
        self.line = 1
        self.column = 1
        self.KEYWORDS = {
            "atur": TokenType.ATUR, "jika": TokenType.JIKA, "maka": TokenType.MAKA, "tidak": TokenType.TIDAK,
            "berhasil": TokenType.BERHASIL, "proses": TokenType.PROSES, "ambil": TokenType.AMBIL,
            "dari": TokenType.DARI, "kembali": TokenType.KEMBALI, "jalankan": TokenType.JALANKAN,
            "tunggu": TokenType.TUNGGU, "pemicu": TokenType.PEMICU, "terjadi": TokenType.TERJADI,
            "harus": TokenType.HARUS, "benar": TokenType.BENAR, "salah": TokenType.SALAH,
            "peta": TokenType.PETA, "ulangi": TokenType.ULANGI, "sebanyak": TokenType.SEBANYAK, "kali": TokenType.KALI,
        }

    def scan_tokens(self) -> list[Token]:
        while not self._is_at_end():
            self.start = self.current
            self._scan_token()
        self.tokens.append(Token(TokenType.ADS, "", self.line, self.column))
        return self.tokens

    def _scan_token(self):
        char = self._advance()

        if char in [' ', '\r', '\t']:
            pass
        elif char == '\n':
            self.line += 1
            self.column = 1
        elif char == '(': self._add_token(TokenType.KURUNG_BUKA)
        elif char == ')': self._add_token(TokenType.KURUNG_TUTUP)
        elif char == '{': self._add_token(TokenType.KURAWAL_BUKA)
        elif char == '}': self._add_token(TokenType.KURAWAL_TUTUP)
        elif char == '[': self._add_token(TokenType.KURUNG_SIKU_BUKA)
        elif char == ']': self._add_token(TokenType.KURUNG_SIKU_TUTUP)
        elif char == ',': self._add_token(TokenType.KOMA)
        elif char == '.': self._add_token(TokenType.TITIK)
        elif char == ';': self._add_token(TokenType.TITIK_KOMA)
        elif char == '+': self._add_token(TokenType.PLUS)
        elif char == '-': self._add_token(TokenType.MINUS)
        elif char == '*': self._add_token(TokenType.BINTANG)
        elif char == '/': self._add_token(TokenType.GARIS_MIRING)
        elif char == '=':
            if self._match('='): self._add_token(TokenType.SAMA_DENGAN_SAMA_DENGAN)
            else: self._add_token(TokenType.SAMA_DENGAN)
        elif char == '!':
            if self._match('='): self._add_token(TokenType.TIDAK_SAMA_DENGAN)
        elif char == '>': self._add_token(TokenType.LEBIH_DARI)
        elif char == '<': self._add_token(TokenType.KURANG_DARI)
        elif char == ':': self._add_token(TokenType.TITIK_DUA)
        elif char == '#':
            while self._peek() != '\n' and not self._is_at_end(): self._advance()
        elif char == '"':
            if self._peek() == '"' and self._peek_next() == '"':
                self._advance() # Lewati " kedua
                self._advance() # Lewati " ketiga
                self._multiline_string()
            else:
                self._string()
        elif self._is_digit(char): self._number()
        elif self._is_alpha(char): self._identifier()
        else:
            self.errors.append(LexerError(f"Karakter tidak dikenali: '{char}'", self.line, self.column - 1))

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
        start_line = self.line
        start_column = self.column - 1
        value = []
        while self._peek() != '"' and not self._is_at_end():
            char = self._peek()
            if char == '\\': # Escape sequence
                self._advance() # Lewati '\'
                if self._is_at_end():
                    self.errors.append(LexerError("String tidak ditutup setelah escape sequence.", self.line, self.column))
                    return
                escaped_char = self._advance()
                if escaped_char == 'n': value.append('\n')
                elif escaped_char == 't': value.append('\t')
                elif escaped_char == '"': value.append('"')
                elif escaped_char == '\\': value.append('\\')
                else:
                    self.errors.append(LexerError(f"Escape sequence tidak valid: '\\{escaped_char}'", self.line, self.column - 2))
            elif char == '\n':
                # String satu baris biasa tidak boleh mengandung newline literal
                self.errors.append(LexerError("Newline tidak diizinkan di dalam string biasa.", self.line, self.column))
                self._advance()
            else:
                value.append(self._advance())

        if self._is_at_end():
            self.errors.append(LexerError("String tidak ditutup.", start_line, start_column))
            return

        self._advance() # Lewati '"' penutup
        self._add_token(TokenType.STRING, "".join(value))

    def _multiline_string(self):
        start_line = self.line
        start_column = self.column - 3
        value = []
        while not (self._peek() == '"' and self._peek_next() == '"' and self.source[self.current + 2] == '"') and not self._is_at_end():
            if self._peek() == '\n':
                self.line += 1
                self.column = 0 # akan di-increment
            value.append(self._advance())

        if self._is_at_end():
            self.errors.append(LexerError("String multi-baris tidak ditutup.", start_line, start_column))
            return

        self._advance() # "
        self._advance() # "
        self._advance() # "

        self._add_token(TokenType.STRING, "".join(value))


    def _is_at_end(self):
        return self.current >= len(self.source)

    def _advance(self):
        char = self.source[self.current]
        self.current += 1
        self.column += 1
        return char

    def _add_token(self, type, literal=None):
        text = self.source[self.start:self.current]
        col = self.column - (self.current - self.start)
        if literal is None:
            literal = text
        self.tokens.append(Token(type, literal, self.line, col))

    def _match(self, expected: str) -> bool:
        if self._is_at_end(): return False
        if self.source[self.current] != expected: return False
        self.current += 1
        self.column += 1
        return True

    def _peek(self):
        if self._is_at_end(): return '\0'
        return self.source[self.current]

    def _peek_next(self):
        if self.current + 1 >= len(self.source): return '\0'
        return self.source[self.current + 1]

    def _is_digit(self, char):
        return '0' <= char <= '9'

    def _is_alpha(self, char):
        return ('a' <= char <= 'z') or ('A' <= char <= 'Z') or char == '_'

    def _is_alpha_numeric(self, char):
        return self._is_alpha(char) or self._is_digit(char)
