# interpreter/parser.py
from .token_types import TokenType
from .token import Token
import interpreter.ast_nodes as ast
from typing import List

class ParseError(Exception):
    pass

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = [t for t in tokens if t.type != TokenType.TIDAK_DIKENALI]
        self.current = 0
        self.errors: List[str] = []

    def parse(self) -> ast.Program:
        statements = []
        while not self._is_at_end():
            try:
                stmt = self._statement()
                if stmt: statements.append(stmt)
            except ParseError:
                self._synchronize()
        return ast.Program(statements=statements)

    def _statement(self) -> ast.Statement:
        if self._match(TokenType.ULANGI): return self._ulangi_statement()
        if self._match(TokenType.PROSES): return self._proses_statement()
        if self._match(TokenType.KEMBALI): return self._kembali_statement()
        if self._match(TokenType.KURAWAL_BUKA): return ast.BlokStatement(self._blok())
        if self._match(TokenType.JIKA): return self._jika_statement()
        if self._match(TokenType.ATUR): return self._atur_statement()
        return self._expression_statement()

    # ... (metode statement lainnya tetap sama) ...
    def _proses_statement(self) -> ast.ProsesStatement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama proses.")
        self._consume(TokenType.KURUNG_BUKA, "Diharapkan '(' setelah nama proses.")
        params = []
        if not self._check(TokenType.KURUNG_TUTUP):
            params.append(self._consume(TokenType.IDENTIFIER, "Diharapkan nama parameter."))
            while self._match(TokenType.KOMA):
                params.append(self._consume(TokenType.IDENTIFIER, "Diharapkan nama parameter."))
        self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')' setelah parameter.")
        self._consume(TokenType.KURAWAL_BUKA, "Diharapkan '{' sebelum badan proses.")
        body = ast.BlokStatement(self._blok())
        return ast.ProsesStatement(name, params, body)

    def _kembali_statement(self) -> ast.KembaliStatement:
        keyword = self._previous()
        value = None
        if self._peek().type != TokenType.EOF and self._peek().type != TokenType.KURAWAL_TUTUP:
            value = self._expression()
        return ast.KembaliStatement(keyword, value)

    def _blok(self) -> List[ast.Statement]:
        statements = []
        while not self._check(TokenType.KURAWAL_TUTUP) and not self._is_at_end():
            statements.append(self._statement())
        self._consume(TokenType.KURAWAL_TUTUP, "Diharapkan '}' setelah blok.")
        return statements

    def _jika_statement(self) -> ast.Statement:
        atur_stmt = self._atur_statement()
        if not isinstance(atur_stmt, ast.AturStatement):
            raise self._error(self._peek(), "Hanya 'atur' yang didukung dalam 'jika' saat ini.")
        self._consume(TokenType.TIDAK, "Diharapkan 'tidak'.")
        self._consume(TokenType.BERHASIL, "Diharapkan 'berhasil'.")
        self._consume(TokenType.MAKA, "Diharapkan 'maka'.")
        fallback_stmt = self._statement()
        if not isinstance(fallback_stmt, ast.AturStatement):
             raise self._error(self._peek(), "Fallback harus berupa 'atur' saat ini.")
        atur_stmt.fallback = fallback_stmt
        return atur_stmt

    def _ulangi_statement(self) -> ast.Statement:
        body = self._statement()
        self._consume(TokenType.SEBANYAK, "Diharapkan 'sebanyak' setelah badan perulangan.")
        count = self._expression()
        self._consume(TokenType.KALI, "Diharapkan 'kali' setelah jumlah perulangan.")
        return ast.UlangiStatement(count=count, body=body)

    def _atur_statement(self) -> ast.AturStatement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama variabel.")
        self._consume(TokenType.SAMA_DENGAN, "Diharapkan '='.")
        initializer = self._expression()
        return ast.AturStatement(name=name, initializer=initializer)

    def _expression_statement(self) -> ast.Statement:
        expr = self._expression()
        return ast.ExpressionStatement(expression=expr)


    # --- HIERARKI PRESEDEN EKSPRESI (YANG DIPERBAIKI) ---
    def _expression(self) -> ast.Expression:
        return self._equality()

    def _equality(self) -> ast.Expression:
        expr = self._comparison()
        while self._match(TokenType.TIDAK_SAMA_DENGAN, TokenType.SAMA_DENGAN_SAMA_DENGAN):
            operator = self._previous()
            right = self._comparison()
            expr = ast.BinaryExpression(left=expr, operator=operator, right=right)
        return expr

    def _comparison(self) -> ast.Expression:
        expr = self._term() # Diubah dari _addition
        while self._match(TokenType.LEBIH_DARI, TokenType.KURANG_DARI):
            operator = self._previous()
            right = self._term() # Diubah dari _addition
            expr = ast.BinaryExpression(left=expr, operator=operator, right=right)
        return expr

    def _term(self) -> ast.Expression: # Dulu bernama _addition
        expr = self._factor() # Diubah dari _multiplication
        while self._match(TokenType.PLUS, TokenType.MINUS):
            operator = self._previous()
            right = self._factor() # Diubah dari _multiplication
            expr = ast.BinaryExpression(left=expr, operator=operator, right=right)
        return expr

    def _factor(self) -> ast.Expression: # Dulu bernama _multiplication
        expr = self._unary()
        while self._match(TokenType.BINTANG, TokenType.GARIS_MIRING):
            operator = self._previous()
            right = self._unary()
            expr = ast.BinaryExpression(left=expr, operator=operator, right=right)
        return expr

    def _unary(self) -> ast.Expression:
        if self._match(TokenType.MINUS):
            operator = self._previous()
            right = self._unary()
            return ast.UnaryExpression(operator=operator, right=right)
        return self._call()

    # ... (sisa metode _call, _primary, dll. tetap sama) ...
    def _call(self) -> ast.Expression:
        expr = self._primary()
        while True:
            if self._match(TokenType.KURUNG_BUKA):
                expr = self._finish_call(expr)
            elif self._match(TokenType.TITIK):
                name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama properti setelah '.'")
                expr = ast.GetExpression(objek=expr, name=name)
            elif self._match(TokenType.KURUNG_SIKU_BUKA):
                indeks = self._expression()
                self._consume(TokenType.KURUNG_SIKU_TUTUP, "Diharapkan ']' setelah indeks.")
                expr = ast.SubscriptExpression(objek=expr, indeks=indeks)
            else:
                break
        return expr

    def _finish_call(self, callee: ast.Expression) -> ast.Expression:
        arguments = []
        if not self._check(TokenType.KURUNG_TUTUP):
            arguments.append(self._expression())
            while self._match(TokenType.KOMA):
                arguments.append(self._expression())
        self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')' setelah argumen.")
        return ast.FunctionCall(callee=callee, arguments=arguments)

    def _primary(self) -> ast.Expression:
        if self._match(TokenType.NUMBER, TokenType.STRING, TokenType.BENAR, TokenType.SALAH):
            return ast.Literal(value=self._previous().literal)
        if self._match(TokenType.IDENTIFIER):
            return ast.Variable(name=self._previous())
        if self._match(TokenType.KURUNG_BUKA):
            expr = self._expression()
            self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')' setelah ekspresi.")
            return ast.Grouping(expression=expr)
        if self._match(TokenType.AMBIL):
            keyword = self._previous()
            self._consume(TokenType.DARI, "Diharapkan 'dari'.")
            path = self._primary()
            if not isinstance(path, ast.Literal) or not isinstance(path.value, str):
                raise self._error(keyword, "Path modul harus string.")
            return ast.AmbilExpression(keyword=keyword, path=path)
        if self._match(TokenType.KURUNG_SIKU_BUKA):
            elements = []
            if not self._check(TokenType.KURUNG_SIKU_TUTUP):
                elements.append(self._expression())
                while self._match(TokenType.KOMA):
                    elements.append(self._expression())
            self._consume(TokenType.KURUNG_SIKU_TUTUP, "Diharapkan ']' setelah elemen daftar.")
            return ast.ListLiteral(elements=elements)
        if self._match(TokenType.PETA):
            keys = []
            values = []
            self._consume(TokenType.KURAWAL_BUKA, "Diharapkan '{' setelah 'peta'.")
            if not self._check(TokenType.KURAWAL_TUTUP):
                key = self._expression()
                self._consume(TokenType.TITIK_DUA, "Diharapkan ':' setelah kunci peta.")
                value = self._expression()
                keys.append(key)
                values.append(value)
                while self._match(TokenType.KOMA):
                    key = self._expression()
                    self._consume(TokenType.TITIK_DUA, "Diharapkan ':' setelah kunci peta.")
                    value = self._expression()
                    keys.append(key)
                    values.append(value)
            self._consume(TokenType.KURAWAL_TUTUP, "Diharapkan '}' setelah elemen peta.")
            return ast.MapLiteral(keys=keys, values=values)
        raise self._error(self._peek(), "Diharapkan sebuah ekspresi.")

    def _match(self, *types: TokenType) -> bool:
        for type in types:
            if self._check(type):
                self._advance()
                return True
        return False

    def _check(self, type: TokenType) -> bool:
        if self._is_at_end(): return False
        return self._peek().type == type

    def _advance(self) -> Token:
        if not self._is_at_end(): self.current += 1
        return self._previous()

    def _is_at_end(self) -> bool:
        return self._peek().type == TokenType.EOF

    def _peek(self) -> Token:
        return self.tokens[self.current]

    def _previous(self) -> Token:
        return self.tokens[self.current - 1]

    def _consume(self, type: TokenType, message: str) -> Token:
        if self._check(type): return self._advance()
        raise self._error(self._peek(), message)

    def _error(self, token: Token, message: str) -> ParseError:
        error_message = f"Error di baris {token.line}: {message}"
        self.errors.append(error_message)
        return ParseError(error_message)

    def _synchronize(self):
        self._advance()
        while not self._is_at_end():
            # Bisa ditambahkan logika sinkronisasi yang lebih baik di sini
            if self._previous().type in [TokenType.ATUR, TokenType.PROSES, TokenType.JIKA, TokenType.ULANGI, TokenType.KEMBALI]:
                return
            self._advance()
