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
        if self._match(TokenType.ATUR):
            return self._atur_statement()
        else:
            return self._expression_statement()

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

        if self._match(TokenType.KURUNG_BUKA):
            # Jika kita menemukan '(', kita selesaikan pemanggilan fungsi
            return self._finish_call(expr)

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
