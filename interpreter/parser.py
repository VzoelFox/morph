from .token_types import TokenType
from .token import Token
import interpreter.ast_nodes as ast

class Parser:
    def __init__(self, tokens: list[Token]):
        self.tokens = [t for t in tokens if t.type != TokenType.TIDAK_DIKENALI]
        self.current = 0

    def parse(self) -> ast.Program:
        statements = []
        while not self._is_at_end():
            statements.append(self._statement())
        return ast.Program(statements=statements)

    def _statement(self):
        if self._match(TokenType.PROSES): return self._proses_statement()
        if self._match(TokenType.KEMBALI): return self._kembali_statement()
        if self._match(TokenType.JIKA): return self._jika_statement()
        if self._match(TokenType.ATUR): return self._atur_statement()
        if self._match(TokenType.KURAWAL_BUKA): return ast.BlokStatement(self._blok())
        return self._expression_statement()

    def _proses_statement(self):
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama proses.")
        self._consume(TokenType.KURUNG_BUKA, "Diharapkan '('.")
    def _statement(self) -> ast.Statement:
        if self._match(TokenType.PROSES): return self._proses_statement()
        if self._match(TokenType.KEMBALI): return self._kembali_statement()
        if self._match(TokenType.KURAWAL_BUKA): return ast.BlokStatement(self._blok())
        if self._match(TokenType.JIKA): return self._jika_statement()
        if self._match(TokenType.ATUR): return self._atur_statement()
        return self._expression_statement()
    def _proses_statement(self) -> ast.ProsesStatement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama proses.")
        self._consume(TokenType.KURUNG_BUKA, "Diharapkan '(' setelah nama proses.")
        params = []
        if not self._check(TokenType.KURUNG_TUTUP):
            params.append(self._consume(TokenType.IDENTIFIER, "Diharapkan nama parameter."))
            while self._match(TokenType.KOMA):
                params.append(self._consume(TokenType.IDENTIFIER, "Diharapkan nama parameter."))
        self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')'.")
        self._consume(TokenType.KURAWAL_BUKA, "Diharapkan '{'.")
        body = ast.BlokStatement(self._blok())
        return ast.ProsesStatement(name, params, body)

    def _kembali_statement(self):
        keyword = self._previous()
        value = None
        if not self._check(TokenType.KURAWAL_TUTUP):
            value = self._expression()
        return ast.KembaliStatement(keyword, value)

    def _jika_statement(self):
        condition = self._expression()
        self._consume(TokenType.MAKA, "Diharapkan 'maka'.")
        then_branch = self._statement()
        # (else branch akan ditambahkan nanti)
        return ast.JikaStatement(condition, then_branch) # Perlu AST node baru

    def _atur_statement(self):
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama variabel.")
        self._consume(TokenType.SAMA_DENGAN, "Diharapkan '='.")
        initializer = self._expression()
        return ast.AturStatement(name, initializer)

    def _expression_statement(self):
        expr = self._expression()
        return ast.ExpressionStatement(expr)

    def _blok(self):
        statements = []
        while not self._check(TokenType.KURAWAL_TUTUP) and not self._is_at_end():
            statements.append(self._statement())
        self._consume(TokenType.KURAWAL_TUTUP, "Diharapkan '}'.")
        return statements

    # Expression Parsing
    def _expression(self): return self._equality()
    def _equality(self):
        expr = self._comparison()
        while self._match(TokenType.TIDAK_SAMA_DENGAN, TokenType.SAMA_DENGAN_SAMA_DENGAN):
            op = self._previous(); right = self._comparison()
            expr = ast.BinaryExpression(expr, op, right)
        return expr
    def _comparison(self):
        expr = self._term()
        while self._match(TokenType.LEBIH_DARI, TokenType.LEBIH_DARI_SAMA_DENGAN, TokenType.KURANG_DARI, TokenType.KURANG_DARI_SAMA_DENGAN):
            op = self._previous(); right = self._term()
            expr = ast.BinaryExpression(expr, op, right)
        return expr
    def _term(self):
        expr = self._factor()
        while self._match(TokenType.MINUS, TokenType.PLUS):
            op = self._previous(); right = self._factor()
            expr = ast.BinaryExpression(expr, op, right)
        return expr
    def _factor(self):
        expr = self._unary()
        while self._match(TokenType.GARIS_MIRING, TokenType.BINTANG):
            op = self._previous(); right = self._unary()
            expr = ast.BinaryExpression(expr, op, right)
        return expr
    def _unary(self):
        if self._match(TokenType.TIDAK_LOGIS, TokenType.MINUS):
            op = self._previous(); right = self._unary()
            return ast.UnaryExpression(op, right)
        return self._call()
    def _call(self):
        expr = self._primary()
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
    def _blok(self) -> list[ast.Statement]:
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
    def _atur_statement(self) -> ast.AturStatement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama variabel.")
        self._consume(TokenType.SAMA_DENGAN, "Diharapkan '='.")
        initializer = self._expression()
        return ast.AturStatement(name=name, initializer=initializer)
    def _expression_statement(self) -> ast.Statement:
        expr = self._expression()
        return ast.ExpressionStatement(expression=expr)
    def _expression(self) -> ast.Expression:
        return self._addition()
    def _addition(self) -> ast.Expression:
        expr = self._call()
        while self._match(TokenType.PLUS):
            operator = self._previous()
            right = self._call()
            expr = ast.BinaryExpression(left=expr, operator=operator, right=right)
        return expr
    def _call(self) -> ast.Expression:
        expr = self._primary()

        return ast.Program(statements=statements)

    def _statement(self) -> ast.Statement:
        if self._match(TokenType.JIKA):
            # Ini adalah blok vzoelinfinity
            return self._jika_statement()
        elif self._match(TokenType.ATUR):
        if self._match(TokenType.ATUR):
            return self._atur_statement()
        else:
            return self._expression_statement()

    def _jika_statement(self) -> ast.Statement:
        # Format: jika var = expr tidak berhasil maka stmt
        # Kita parse bagian 'atur' terlebih dahulu
        atur_stmt = self._atur_statement()

        self._consume(TokenType.TIDAK, "Diharapkan 'tidak' setelah ekspresi dalam blok 'jika'.")
        self._consume(TokenType.BERHASIL, "Diharapkan 'berhasil' setelah 'tidak'.")
        self._consume(TokenType.MAKA, "Diharapkan 'maka' setelah 'tidak berhasil'.")

        fallback_stmt = self._statement()

        # Pastikan fallback adalah atur_statement juga untuk saat ini
        if not isinstance(fallback_stmt, ast.AturStatement):
             raise self._error(self._peek(), "Fallback harus berupa pernyataan 'atur' saat ini.")

        atur_stmt.fallback = fallback_stmt
        return atur_stmt

    def _atur_statement(self) -> ast.AturStatement:
    def _atur_statement(self) -> ast.Statement:
        name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama variabel.")
        self._consume(TokenType.SAMA_DENGAN, "Diharapkan '=' setelah nama variabel.")
        initializer = self._expression()
        return ast.AturStatement(name=name, initializer=initializer)

    def _expression_statement(self) -> ast.Statement:
        expr = self._expression()
        return ast.ExpressionStatement(expression=expr)

    def _expression(self) -> ast.Expression:
        return self._call()

    def _call(self) -> ast.Expression:
        expr = self._primary()

        while True:
            if self._match(TokenType.KURUNG_BUKA):
                expr = self._finish_call(expr)
            elif self._match(TokenType.TITIK):
                name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama properti.")
                expr = ast.GetExpression(expr, name)
            else: break
        return expr
    def _finish_call(self, callee):
        args = []
        if not self._check(TokenType.KURUNG_TUTUP):
            args.append(self._expression())
            while self._match(TokenType.KOMA):
                args.append(self._expression())
        self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')'.")
        return ast.FunctionCall(callee, args)
    def _primary(self):
        if self._match(TokenType.BENAR): return ast.Literal(True)
        if self._match(TokenType.SALAH): return ast.Literal(False)
        if self._match(TokenType.NUMBER, TokenType.STRING): return ast.Literal(self._previous().literal)
        if self._match(TokenType.IDENTIFIER): return ast.Variable(self._previous())
        if self._match(TokenType.KURUNG_BUKA):
            expr = self._expression()
            self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')'.")
            return expr # (Grouping expression)
        raise Exception("Diharapkan sebuah ekspresi.")

    # Helpers
    def _match(self, *types):
        for type in types:
            if self._check(type): self._advance(); return True
        return False
    def _check(self, type):
        if self._is_at_end(): return False
        return self._peek().type == type
    def _advance(self):
        if not self._is_at_end(): self.current += 1
        return self._previous()
    def _previous(self): return self.tokens[self.current - 1]
    def _is_at_end(self): return self._peek().type == TokenType.EOF
    def _peek(self): return self.tokens[self.current]
    def _consume(self, type, message):
        if self._check(type): return self._advance()
        raise Exception(message)
                expr = ast.GetExpression(objek=expr, name=name)
            else: break
        return expr
                name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama properti setelah '.'.")
                expr = ast.GetExpression(objek=expr, name=name)
            else:
                break
        if self._match(TokenType.KURUNG_BUKA):
            # Jika kita menemukan '(', kita selesaikan pemanggilan fungsi
            return self._finish_call(expr)

        return expr

    def _finish_call(self, callee: ast.Expression) -> ast.FunctionCall:
        arguments = []
        if not self._check(TokenType.KURUNG_TUTUP):
            arguments.append(self._expression())
            while self._match(TokenType.KOMA):
                arguments.append(self._expression())
        self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')'.")
        if not isinstance(callee, (ast.Variable, ast.GetExpression)):
            raise self._error(self._peek(), "Target pemanggilan tidak valid.")
        return ast.FunctionCall(callee=callee, arguments=arguments)
    def _primary(self) -> ast.Expression:
        if self._match(TokenType.NUMBER, TokenType.STRING, TokenType.BENAR, TokenType.SALAH):
            return ast.Literal(value=self._previous().literal)
        if self._match(TokenType.AMBIL):
            keyword = self._previous()
            self._consume(TokenType.DARI, "Diharapkan 'dari'.")
            path = self._primary()
            if not isinstance(path, ast.Literal) or not isinstance(path.value, str):
                raise self._error(keyword, "Path modul harus string.")
            return ast.AmbilExpression(keyword=keyword, path=path)
        if self._match(TokenType.IDENTIFIER):
            return ast.Variable(name=self._previous())
        raise self._error(self._peek(), "Diharapkan sebuah ekspresi.")
    def _match(self, *types: TokenType) -> bool:
        for type in types:
            if self._check(type):
                self._advance(); return True
        return False
    def _check(self, type: TokenType) -> bool:
        if self._is_at_end(): return False
        return self._peek().type == type
    def _advance(self) -> Token:
        if not self._is_at_end(): self.current += 1
        return self._previous()
    def _is_at_end(self) -> bool: return self._peek().type == TokenType.EOF
    def _peek(self) -> Token: return self.tokens[self.current]
    def _previous(self) -> Token: return self.tokens[self.current - 1]
    def _consume(self, type: TokenType, message: str) -> Token:
        if self._check(type): return self._advance()
        raise self._error(self._peek(), message)
    def _error(self, token: Token, message: str):
            # Di sini kita bisa menambahkan loop untuk multi-argumen nanti

        self._consume(TokenType.KURUNG_TUTUP, "Diharapkan ')' setelah argumen.")

        # Pastikan callee adalah sesuatu yang bisa dipanggil (seperti variabel)
        if not isinstance(callee, ast.Variable):
            raise self._error(self._peek(), "Target pemanggilan tidak valid.")

        return ast.FunctionCall(callee=callee, arguments=arguments)

    def _primary(self) -> ast.Expression:
        if self._match(TokenType.NUMBER, TokenType.STRING, TokenType.BENAR, TokenType.SALAH):
            return ast.Literal(value=self._previous().literal)

        if self._match(TokenType.AMBIL):
            keyword = self._previous()
            self._consume(TokenType.DARI, "Diharapkan 'dari' setelah 'ambil'.")
            path = self._primary() # Harusnya string literal
            if not isinstance(path, ast.Literal) or not isinstance(path.value, str):
                raise self._error(keyword, "Path modul harus berupa string literal.")
            return ast.AmbilExpression(keyword=keyword, path=path)

        if self._match(TokenType.IDENTIFIER):
            return ast.Variable(name=self._previous())

        raise self._error(self._peek(), "Diharapkan sebuah ekspresi.")


    # --- Helper Methods ---
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

    def _error(self, token: Token, message: str):
        # Nanti kita bisa buat error reporting yang lebih baik
        return Exception(f"Error di baris {token.line}: {message}")
