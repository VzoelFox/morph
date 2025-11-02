# interpreter/parser.py
from .token_types import TokenType
from .token import Token
from .errors import ParserError
import interpreter.ast_nodes as ast
from typing import List

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = [t for t in tokens if t.type != TokenType.TIDAK_DIKENALI]
        self.current = 0
        self.errors: List[ParserError] = []

    def parse(self) -> ast.Program:
        statements = []
        while not self._is_at_end():
            stmt = self._declaration()
            if stmt:
                statements.append(stmt)
        return ast.Program(statements=statements)

    def _declaration(self):
        try:
            if self._match(TokenType.MANAGEMENT): return self._management_statement()
            if self._match(TokenType.PROSES): return self._proses_statement("proses")
            if self._match(TokenType.JIKA): return self._jika_statement()
            if self._match(TokenType.ATUR): return self._atur_statement()
            return self._statement()
        except ParserError:
            self._synchronize()
            return None

    def _statement(self) -> ast.Statement:
        if self._match(TokenType.JALANKAN): return self._jalankan_statement()
        if self._match(TokenType.ULANGI): return self._ulangi_statement()
        if self._match(TokenType.KEMBALI): return self._kembali_statement()
        if self._match(TokenType.KURAWAL_BUKA): return ast.BlokStatement(self._blok())
        return self._expression_statement()

    def _proses_statement(self, kind: str) -> ast.ProsesStatement:
        name = self._consume(TokenType.IDENTIFIER, f"Diharapkan nama {kind}.")
        self._consume(TokenType.KURUNG_BUKA, f"Diharapkan '(' setelah nama {kind}.")
        params = []
        if not self._check(TokenType.KURUNG_TUTUP):
            params.append(self._consume(TokenType.IDENTIFIER, "Diharapkan nama parameter."))
            while self._match(TokenType.KOMA):
                params.append(self._consume(TokenType.IDENTIFIER, "Diharapkan nama parameter."))
        self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')' setelah parameter.")
        self._consume(TokenType.KURAWAL_BUKA, f"Diharapkan '{{' sebelum badan {kind}.")
        body = ast.BlokStatement(self._blok())
        return ast.ProsesStatement(name, params, body)

    def _kembali_statement(self) -> ast.KembaliStatement:
        keyword = self._previous()
        value = None
        if not self._check(TokenType.ADS) and self._peek().type != TokenType.KURAWAL_TUTUP:
            value = self._expression()
        self._match(TokenType.TITIK_KOMA) # Opsional
        return ast.KembaliStatement(keyword, value)

    def _blok(self) -> List[ast.Statement]:
        statements = []
        while not self._check(TokenType.KURAWAL_TUTUP) and not self._is_at_end():
            statements.append(self._declaration())
        self._consume(TokenType.KURAWAL_TUTUP, "Diharapkan '}}' setelah blok.")
        return statements

    def _jika_statement(self) -> ast.Statement:
        self._consume(TokenType.ATUR, "Diharapkan 'atur' setelah 'jika'.")
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama variabel.")
        self._consume(TokenType.SAMA_DENGAN, "Diharapkan '='.")
        initializer = self._expression()
        atur_stmt = ast.AturStatement(name=name, initializer=initializer)

        self._consume(TokenType.TIDAK, "Diharapkan 'tidak'.")
        self._consume(TokenType.BERHASIL, "Diharapkan 'berhasil'.")
        self._consume(TokenType.MAKA, "Diharapkan 'maka'.")

        fallback_stmt = self._declaration()
        atur_stmt.fallback = fallback_stmt
        return atur_stmt

    def _ulangi_statement(self) -> ast.Statement:
        body = self._statement()
        self._consume(TokenType.SEBANYAK, "Diharapkan 'sebanyak' setelah 'ulangi'.")
        count = self._expression()
        self._consume(TokenType.KALI, "Diharapkan 'kali' setelah jumlah perulangan.")
        self._match(TokenType.TITIK_KOMA) # Opsional
        return ast.UlangiStatement(count=count, body=body)

    def _atur_statement(self) -> ast.AturStatement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama variabel.")
        self._consume(TokenType.SAMA_DENGAN, "Diharapkan '='.")
        initializer = self._expression()
        self._match(TokenType.TITIK_KOMA) # Opsional
        return ast.AturStatement(name=name, initializer=initializer)

    def _expression_statement(self) -> ast.Statement:
        expr = self._expression()
        self._match(TokenType.TITIK_KOMA) # Opsional
        return ast.ExpressionStatement(expression=expr)

    def _jalankan_statement(self) -> ast.Statement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama sistem management untuk dijalankan.")
        self._match(TokenType.TITIK_KOMA) # Opsional
        return ast.JalankanStatement(name=name)

    def _management_statement(self) -> ast.Statement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama sistem management.")
        self._consume(TokenType.KURAWAL_BUKA, "Diharapkan '{' setelah nama sistem management.")
        bagian_list = []
        while self._match(TokenType.BAGIAN):
            bagian_list.append(self._bagian_statement())
        self._consume(TokenType.KURAWAL_TUTUP, "Diharapkan '}' setelah blok management.")
        return ast.ManagementStatement(name=name, bagian=bagian_list)

    def _bagian_statement(self) -> ast.BagianStatement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama bagian.")
        self._consume(TokenType.KURAWAL_BUKA, "Diharapkan '{' setelah nama bagian.")
        pecahan_list = []
        while self._match(TokenType.PECAHAN):
            pecahan_list.append(self._pecahan_statement())
        self._consume(TokenType.KURAWAL_TUTUP, "Diharapkan '}' setelah blok bagian.")
        return ast.BagianStatement(name=name, pecahan=pecahan_list)

    def _pecahan_statement(self) -> ast.PecahanStatement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama pecahan.")
        self._consume(TokenType.KURAWAL_BUKA, "Diharapkan '{' setelah nama pecahan.")
        body = ast.BlokStatement(self._blok())
        return ast.PecahanStatement(name=name, body=body)

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
        expr = self._term()
        while self._match(TokenType.LEBIH_DARI, TokenType.KURANG_DARI):
            operator = self._previous()
            right = self._term()
            expr = ast.BinaryExpression(left=expr, operator=operator, right=right)
        return expr
    def _term(self) -> ast.Expression:
        expr = self._factor()
        while self._match(TokenType.PLUS, TokenType.MINUS):
            operator = self._previous()
            right = self._factor()
            expr = ast.BinaryExpression(left=expr, operator=operator, right=right)
        return expr
    def _factor(self) -> ast.Expression:
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
        if self._match(TokenType.SALAH): return ast.Literal(value=False)
        if self._match(TokenType.BENAR): return ast.Literal(value=True)
        if self._match(TokenType.NUMBER, TokenType.STRING):
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
                self._error(self._previous(), "Path modul harus string.")
                raise ParserError("Path modul harus string.")
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
            keys, values = [], []
            self._consume(TokenType.KURAWAL_BUKA, "Diharapkan '{' setelah 'peta'.")
            if not self._check(TokenType.KURAWAL_TUTUP):
                while True:
                    key = self._expression()
                    self._consume(TokenType.TITIK_DUA, "Diharapkan ':' setelah kunci peta.")
                    value = self._expression()
                    keys.append(key)
                    values.append(value)
                    if not self._match(TokenType.KOMA):
                        break
            self._consume(TokenType.KURAWAL_TUTUP, "Diharapkan '}' setelah elemen peta.")
            return ast.MapLiteral(keys=keys, values=values)

        self._error(self._peek(), "Diharapkan sebuah ekspresi.")
        raise ParserError("Diharapkan sebuah ekspresi.", self._peek().line, self._peek().column)

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
        return self._peek().type == TokenType.ADS
    def _peek(self) -> Token:
        return self.tokens[self.current]
    def _previous(self) -> Token:
        return self.tokens[self.current - 1]
    def _consume(self, type: TokenType, message: str) -> Token:
        if self._check(type): return self._advance()
        self._error(self._peek(), message)
        raise ParserError(message, self._peek().line, self._peek().column)
    def _error(self, token: Token, message: str):
        if token.type == TokenType.ADS:
            self.errors.append(ParserError(f"di akhir file: {message}", token.line, token.column))
        else:
            self.errors.append(ParserError(f"pada '{token.literal}': {message}", token.line, token.column))
    def _synchronize(self):
        self._advance()
        while not self._is_at_end():
            if self._previous().type == TokenType.TITIK_KOMA: return
            sync_tokens = [
                TokenType.PROSES, TokenType.ATUR, TokenType.JIKA,
                TokenType.ULANGI, TokenType.KEMBALI,
            ]
            if self._peek().type in sync_tokens:
                return
            self._advance()
