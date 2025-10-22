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
