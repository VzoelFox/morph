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

    def _statement(self) -> ast.Statement:
        if self._match(TokenType.JIKA):
            # Ini adalah blok vzoelinfinity
            return self._jika_statement()
        elif self._match(TokenType.ATUR):
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
                name = self._consume(TokenType.IDENTIFIER, "Diharapkan nama properti setelah '.'.")
                expr = ast.GetExpression(objek=expr, name=name)
            else:
                break

        return expr

    def _finish_call(self, callee: ast.Expression) -> ast.FunctionCall:
        arguments = []
        if not self._check(TokenType.KURUNG_TUTUP):
            arguments.append(self._expression())
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
