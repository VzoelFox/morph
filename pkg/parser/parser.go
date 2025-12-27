package parser

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/VzoelFox/morph/pkg/lexer"
)

const (
	_ int = iota
	LOWEST
	OR          // atau
	AND         // dan
	EQUALS      // ==
	LESSGREATER // > or <
	BITOR       // |
	BITXOR      // ^
	BITAND      // &
	SHIFT       // << or >>
	SUM         // +
	PRODUCT     // * or / or %
	PREFIX      // -X or !X or ~X
	CALL        // myFunction(X)
	INDEX       // array[index]
)

// Maximum recursion depth to prevent stack overflow from deeply nested expressions
const MAX_RECURSION_DEPTH = 1000

var precedences = map[lexer.TokenType]int{
	lexer.ATAU:     OR,
	lexer.DAN:      AND,
	lexer.OR:       BITOR,
	lexer.XOR:      BITXOR,
	lexer.AND:      BITAND,
	lexer.EQ:       EQUALS,
	lexer.NOT_EQ:   EQUALS,
	lexer.LT:       LESSGREATER,
	lexer.GT:       LESSGREATER,
	lexer.LTE:      LESSGREATER,
	lexer.GTE:      LESSGREATER,
	lexer.LSHIFT:   SHIFT,
	lexer.RSHIFT:   SHIFT,
	lexer.PLUS:     SUM,
	lexer.MINUS:    SUM,
	lexer.SLASH:    PRODUCT,
	lexer.PERCENT:  PRODUCT,
	lexer.ASTERISK: PRODUCT,
	lexer.LPAREN:   CALL,
	lexer.LBRACKET: INDEX,
	lexer.DOT:      INDEX, // Dot has high precedence
	lexer.LBRACE:   CALL,  // Struct Literal User{...} has Call precedence
}

type (
	prefixParseFn func() Expression
	infixParseFn  func(Expression) Expression
)

type ErrorLevel string

const (
	LEVEL_ERROR   ErrorLevel = "ERROR"
	LEVEL_WARNING ErrorLevel = "WARNING"
	LEVEL_PANIC   ErrorLevel = "PANIC"
)

type ParserError struct {
	Level   ErrorLevel
	Message string
	Line    int
	Column  int
	File    string
	Context string
}

func (e ParserError) String() string {
	pointer := ""
	for i := 0; i < e.Column-1; i++ {
		pointer += " "
	}
	pointer += "^"

	return fmt.Sprintf("%s [%d:%d]: %s\n  %d | %s\n       %s",
		e.Level, e.Line, e.Column, e.Message, e.Line, e.Context, pointer)
}

type Parser struct {
	l      *lexer.Lexer
	errors []ParserError

	curToken   lexer.Token
	peekToken  lexer.Token
	peekToken2 lexer.Token // 2-token lookahead

	curComment   string
	peekComment  string
	peekComment2 string

	prefixParseFns map[lexer.TokenType]prefixParseFn
	infixParseFns  map[lexer.TokenType]infixParseFn
	lineCache      []string

	recursionDepth int // Track recursion depth to prevent stack overflow
}

func New(l *lexer.Lexer) *Parser {
	p := &Parser{
		l:      l,
		errors: []ParserError{},
	}

	p.prefixParseFns = make(map[lexer.TokenType]prefixParseFn)
	p.registerPrefix(lexer.IDENT, p.parseIdentifier)
	p.registerPrefix(lexer.INT, p.parseIntegerLiteral)
	p.registerPrefix(lexer.FLOAT, p.parseFloatLiteral)
	p.registerPrefix(lexer.STRING, p.parseStringLiteral)
	p.registerPrefix(lexer.INTERP_START, p.parseStringLiteral)
	p.registerPrefix(lexer.BENAR, p.parseBoolean)
	p.registerPrefix(lexer.SALAH, p.parseBoolean)
	p.registerPrefix(lexer.KOSONG, p.parseNull)
	p.registerPrefix(lexer.BANG, p.parsePrefixExpression)
	p.registerPrefix(lexer.MINUS, p.parsePrefixExpression)
	p.registerPrefix(lexer.TILDE, p.parsePrefixExpression)
	p.registerPrefix(lexer.AND, p.parsePrefixExpression)
	p.registerPrefix(lexer.ASTERISK, p.parsePrefixExpression)
	p.registerPrefix(lexer.LPAREN, p.parseGroupedExpression)
	p.registerPrefix(lexer.JIKA, p.parseIfExpression)
	p.registerPrefix(lexer.SELAMA, p.parseWhileExpression)
	p.registerPrefix(lexer.FUNGSI, p.parseFunctionLiteral)
	p.registerPrefix(lexer.LBRACKET, p.parseArrayLiteral)
	p.registerPrefix(lexer.LBRACE, p.parseHashLiteral)

	p.infixParseFns = make(map[lexer.TokenType]infixParseFn)
	p.registerInfix(lexer.PLUS, p.parseInfixExpression)
	p.registerInfix(lexer.MINUS, p.parseInfixExpression)
	p.registerInfix(lexer.SLASH, p.parseInfixExpression)
	p.registerInfix(lexer.PERCENT, p.parseInfixExpression)
	p.registerInfix(lexer.ASTERISK, p.parseInfixExpression)
	p.registerInfix(lexer.EQ, p.parseInfixExpression)
	p.registerInfix(lexer.NOT_EQ, p.parseInfixExpression)
	p.registerInfix(lexer.LT, p.parseInfixExpression)
	p.registerInfix(lexer.GT, p.parseInfixExpression)
	p.registerInfix(lexer.LTE, p.parseInfixExpression)
	p.registerInfix(lexer.GTE, p.parseInfixExpression)
	p.registerInfix(lexer.AND, p.parseInfixExpression)
	p.registerInfix(lexer.OR, p.parseInfixExpression)
	p.registerInfix(lexer.XOR, p.parseInfixExpression)
	p.registerInfix(lexer.LSHIFT, p.parseInfixExpression)
	p.registerInfix(lexer.RSHIFT, p.parseInfixExpression)
	p.registerInfix(lexer.DAN, p.parseInfixExpression)
	p.registerInfix(lexer.ATAU, p.parseInfixExpression)
	p.registerInfix(lexer.LPAREN, p.parseCallExpression)
	p.registerInfix(lexer.LBRACKET, p.parseIndexExpression)
	p.registerInfix(lexer.DOT, p.parseDotExpression)
	p.registerInfix(lexer.LBRACE, p.parseStructLiteral)

	// Read tokens to initialize cur, peek, and peek2
	p.nextToken()
	p.nextToken()
	p.nextToken()

	return p
}

func (p *Parser) nextToken() {
	p.curToken = p.peekToken
	p.peekToken = p.peekToken2

	p.curComment = p.peekComment
	p.peekComment = p.peekComment2
	p.peekComment2 = ""

	p.peekToken2 = p.l.NextToken()

	for p.peekToken2.Type == lexer.COMMENT {
		if p.peekComment2 != "" {
			p.peekComment2 += "\n"
		}
		p.peekComment2 += p.peekToken2.Literal
		p.peekToken2 = p.l.NextToken()
	}
}

func (p *Parser) peekToken2Is(t lexer.TokenType) bool {
	return p.peekToken2.Type == t
}

func (p *Parser) Errors() []ParserError {
	return p.errors
}

func (p *Parser) addDetailedError(tok lexer.Token, format string, args ...interface{}) {
	// De-duplicate errors at same location
	if len(p.errors) > 0 {
		lastErr := p.errors[len(p.errors)-1]
		if lastErr.Line == tok.Line && lastErr.Column == tok.Column {
			return
		}
	}

	msg := fmt.Sprintf(format, args...)
	lineContent := p.getLineContent(tok.Line)

	err := ParserError{
		Level:   LEVEL_ERROR,
		Message: msg,
		Line:    tok.Line,
		Column:  tok.Column,
		Context: lineContent,
	}

	p.errors = append(p.errors, err)
}

func (p *Parser) getLineContent(line int) string {
	if p.lineCache == nil {
		p.lineCache = strings.Split(p.l.Input(), "\n")
	}
	if line >= 1 && line <= len(p.lineCache) {
		return p.lineCache[line-1]
	}
	return ""
}

func (p *Parser) peekError(t lexer.TokenType) {
	p.addDetailedError(p.peekToken, "expected next token to be %s, got %s instead", t, p.peekToken.Type)
}

func (p *Parser) curError(t lexer.TokenType) {
	p.addDetailedError(p.curToken, "expected token to be %s, got %s instead", t, p.curToken.Type)
}

func (p *Parser) registerPrefix(tokenType lexer.TokenType, fn prefixParseFn) {
	p.prefixParseFns[tokenType] = fn
}

func (p *Parser) registerInfix(tokenType lexer.TokenType, fn infixParseFn) {
	p.infixParseFns[tokenType] = fn
}

func (p *Parser) ParseProgram() *Program {
	program := &Program{}
	program.Statements = []Statement{}

	for p.curToken.Type != lexer.EOF {
		stmt := p.parseStatement()
		if stmt != nil {
			program.Statements = append(program.Statements, stmt)
		}
		p.nextToken()
	}

	return program
}

func (p *Parser) parseStatement() Statement {
	switch p.curToken.Type {
	case lexer.VAR, lexer.TETAPAN:
		if s := p.parseVarStatement(); s != nil {
			return s
		}
	case lexer.KEMBALIKAN:
		if s := p.parseReturnStatement(); s != nil {
			return s
		}
	case lexer.AMBIL:
		if s := p.parseImportStatement(); s != nil {
			return s
		}
	case lexer.DARI:
		if s := p.parseFromImportStatement(); s != nil {
			return s
		}
	case lexer.STRUKTUR:
		if s := p.parseStructStatement(); s != nil {
			return s
		}
	case lexer.INTERFACE:
		if s := p.parseInterfaceStatement(); s != nil {
			return s
		}
	case lexer.BERHENTI:
		if s := p.parseBreakStatement(); s != nil {
			return s
		}
	case lexer.LANJUT:
		if s := p.parseContinueStatement(); s != nil {
			return s
		}
	case lexer.PILIH:
		if s := p.parseSwitchStatement(); s != nil {
			return s
		}
	default:
		if s := p.parseExpressionOrAssignmentStatement(); s != nil {
			return s
		}
	}
	return nil
}

func (p *Parser) parseSwitchStatement() *SwitchStatement {
	stmt := &SwitchStatement{Token: p.curToken}
	p.nextToken() // eat pilih

	stmt.Condition = p.parseExpression(LOWEST)

	for !p.curTokenIs(lexer.AKHIR) && !p.curTokenIs(lexer.EOF) {
		if p.curTokenIs(lexer.KASUS) {
			caseClause := p.parseCaseClause()
			if caseClause != nil {
				stmt.Cases = append(stmt.Cases, caseClause)
			}
		} else if p.curTokenIs(lexer.LAINNYA) {
			p.nextToken() // eat lainnya
			if p.curTokenIs(lexer.COLON) {
				p.nextToken()
			}
			stmt.Default = p.parseBlockStatement()
		} else {
			p.nextToken()
		}
	}

	if !p.curTokenIs(lexer.AKHIR) {
		p.peekError(lexer.AKHIR)
		return nil
	}

	return stmt
}

func (p *Parser) parseCaseClause() *CaseClause {
	cc := &CaseClause{Token: p.curToken}
	p.nextToken() // eat kasus

	cc.Values = p.parseExpressionList(lexer.COLON)

	if p.curTokenIs(lexer.COLON) {
		p.nextToken()
	}

	cc.Body = p.parseBlockStatement()

	return cc
}

func (p *Parser) parseVarStatement() *VarStatement {
	stmt := &VarStatement{
		Token:   p.curToken,
		Names:   []*Identifier{},
		Values:  []Expression{},
		IsConst: p.curToken.Type == lexer.TETAPAN,
	}

	// Parse first name
	if !p.expectPeek(lexer.IDENT) {
		return nil
	}
	stmt.Names = append(stmt.Names, &Identifier{Token: p.curToken, Value: p.curToken.Literal})

	// Parse subsequent names
	for p.peekTokenIs(lexer.COMMA) {
		p.nextToken()
		if !p.expectPeek(lexer.IDENT) {
			return nil
		}
		stmt.Names = append(stmt.Names, &Identifier{Token: p.curToken, Value: p.curToken.Literal})
	}

	// Parse optional type
	if p.peekIsType() {
		p.nextToken()
		stmt.Type = p.parseType()
		if stmt.Type == nil {
			return nil
		}
	} else {
		stmt.Type = nil
	}

	// Check if assignment follows
	if p.peekTokenIs(lexer.ASSIGN) {
		p.nextToken() // move to ASSIGN
		p.nextToken() // move to Expression start

		// Parse values
		stmt.Values = append(stmt.Values, p.parseExpression(LOWEST))

		for p.peekTokenIs(lexer.COMMA) {
			p.nextToken()
			p.nextToken()
			stmt.Values = append(stmt.Values, p.parseExpression(LOWEST))
		}
	} else if stmt.Type == nil || stmt.IsConst {
		// No type and no assignment -> Error
		// Const MUST have assignment
		if stmt.IsConst {
			p.addDetailedError(p.curToken, "Constant declaration requires value assignment")
		} else {
			p.peekError(lexer.ASSIGN)
		}
		return nil
	}

	if p.peekTokenIs(lexer.SEMICOLON) {
		p.nextToken()
	}

	return stmt
}

func (p *Parser) parseInterfaceStatement() *InterfaceStatement {
	stmt := &InterfaceStatement{Token: p.curToken}

	if !p.expectPeek(lexer.IDENT) {
		return nil
	}
	stmt.Name = &Identifier{Token: p.curToken, Value: p.curToken.Literal}

	p.nextToken() // Move to the start of the interface body

	stmt.Methods = []*FunctionLiteral{}

	for !p.curTokenIs(lexer.AKHIR) && !p.curTokenIs(lexer.EOF) {
		if p.curTokenIs(lexer.FUNGSI) {
			method := p.parseInterfaceMethod()
			if method == nil {
				return nil // Propagate error
			}
			stmt.Methods = append(stmt.Methods, method)
		} else {
			// Skip other tokens until the next function or end
			p.nextToken()
		}
	}

	if !p.curTokenIs(lexer.AKHIR) {
		p.curError(lexer.AKHIR)
		return nil
	}

	return stmt
}

func (p *Parser) parseImportStatement() *ImportStatement {
	stmt := &ImportStatement{Token: p.curToken}

	if !p.expectPeek(lexer.STRING) {
		return nil
	}

	stmt.Path = p.curToken.Literal

	if p.peekTokenIs(lexer.SEMICOLON) {
		p.nextToken()
	}

	return stmt
}

func (p *Parser) parseBreakStatement() *BreakStatement {
	stmt := &BreakStatement{Token: p.curToken}
	if p.peekTokenIs(lexer.SEMICOLON) {
		p.nextToken()
	}
	return stmt
}

func (p *Parser) parseContinueStatement() *ContinueStatement {
	stmt := &ContinueStatement{Token: p.curToken}
	if p.peekTokenIs(lexer.SEMICOLON) {
		p.nextToken()
	}
	return stmt
}

func (p *Parser) parseFromImportStatement() *ImportStatement {
	stmt := &ImportStatement{Token: p.curToken}

	if !p.expectPeek(lexer.STRING) {
		return nil
	}
	stmt.Path = p.curToken.Literal

	if !p.expectPeek(lexer.AMBIL) {
		return nil
	}

	identifiers := []string{}

	if !p.expectPeek(lexer.IDENT) {
		return nil
	}
	identifiers = append(identifiers, p.curToken.Literal)

	for p.peekTokenIs(lexer.COMMA) {
		p.nextToken()
		p.nextToken()
		identifiers = append(identifiers, p.curToken.Literal)
	}

	stmt.Identifiers = identifiers

	if p.peekTokenIs(lexer.SEMICOLON) {
		p.nextToken()
	}

	return stmt
}

func (p *Parser) parseType() TypeNode {
	if p.curTokenIs(lexer.ASTERISK) {
		tok := p.curToken
		p.nextToken()
		elem := p.parseType()
		if elem == nil {
			return nil
		}
		return &PointerType{Token: tok, Element: elem}
	}

	if p.curTokenIs(lexer.LBRACKET) {
		tok := p.curToken
		if !p.expectPeek(lexer.RBRACKET) {
			return nil
		}
		p.nextToken() // eat ]
		elem := p.parseType()
		if elem == nil {
			return nil
		}
		return &ArrayType{Token: tok, Element: elem}
	}

	if p.curTokenIs(lexer.MAP) {
		tok := p.curToken
		if !p.expectPeek(lexer.LBRACKET) {
			return nil
		}
		p.nextToken() // move to key type
		key := p.parseType()
		if key == nil {
			return nil
		}
		if !p.expectPeek(lexer.RBRACKET) {
			return nil
		}
		p.nextToken() // move to value type
		val := p.parseType()
		if val == nil {
			return nil
		}
		return &MapType{Token: tok, Key: key, Value: val}
	}

	if p.curTokenIs(lexer.FUNGSI) {
		tok := p.curToken
		if !p.expectPeek(lexer.LPAREN) {
			return nil
		}
		p.nextToken() // move to first param type or RPAREN

		params := []TypeNode{}
		if !p.curTokenIs(lexer.RPAREN) {
			for {
				pt := p.parseType()
				if pt == nil {
					return nil
				}
				params = append(params, pt)

				if p.peekTokenIs(lexer.COMMA) {
					p.nextToken()
					p.nextToken()
				} else {
					break
				}
			}
		}

		if !p.expectPeek(lexer.RPAREN) {
			return nil
		}

		// Optional return types
		retTypes := []TypeNode{}
		if p.peekTokenIs(lexer.LPAREN) {
			p.nextToken() // move to (
			p.nextToken() // move to first return type

			for {
				rt := p.parseType()
				if rt == nil {
					return nil
				}
				retTypes = append(retTypes, rt)

				if p.peekTokenIs(lexer.COMMA) {
					p.nextToken()
					p.nextToken()
				} else {
					break
				}
			}

			if !p.expectPeek(lexer.RPAREN) {
				return nil
			}
		} else if p.peekIsType() {
			p.nextToken()
			rt := p.parseType()
			if rt == nil {
				return nil
			}
			retTypes = append(retTypes, rt)
		}

		return &FunctionType{Token: tok, Parameters: params, ReturnTypes: retTypes}
	}

	if p.curTokenIs(lexer.IDENT) {
		// Check for Qualified Type: Module.Type
		if p.peekTokenIs(lexer.DOT) {
			pkgIdent := &Identifier{Token: p.curToken, Value: p.curToken.Literal}
			p.nextToken() // eat IDENT
			// cur is DOT
			p.nextToken() // eat DOT
			// cur should be IDENT
			if !p.curTokenIs(lexer.IDENT) {
				p.addDetailedError(p.curToken, "Expected type name after dot")
				return nil
			}
			nameIdent := &Identifier{Token: p.curToken, Value: p.curToken.Literal}
			return &QualifiedType{Token: pkgIdent.Token, Package: pkgIdent, Name: nameIdent}
		}
		return &SimpleType{Token: p.curToken, Name: p.curToken.Literal}
	}

	p.addDetailedError(p.curToken, "expected type, got %s", p.curToken.Type)
	return nil
}

func (p *Parser) parseStructStatement() *StructStatement {
	stmt := &StructStatement{Token: p.curToken}
	stmt.Doc = p.curComment

	if !p.expectPeek(lexer.IDENT) {
		return nil
	}
	stmt.Name = &Identifier{Token: p.curToken, Value: p.curToken.Literal}

	p.nextToken()

	for !p.curTokenIs(lexer.AKHIR) && !p.curTokenIs(lexer.EOF) {
		if p.curTokenIs(lexer.SEMICOLON) {
			p.nextToken()
			continue
		}

		if p.curTokenIs(lexer.IDENT) {
			field := &StructField{Token: p.curToken, Name: p.curToken.Literal}

			p.nextToken() // move to Type start
			field.Type = p.parseType()
			if field.Type == nil {
				return nil
			}

			stmt.Fields = append(stmt.Fields, field)
			p.nextToken()
		} else {
			p.nextToken()
		}
	}

	if !p.curTokenIs(lexer.AKHIR) {
		p.peekError(lexer.AKHIR)
		return nil
	}

	return stmt
}

func (p *Parser) parseReturnStatement() *ReturnStatement {
	stmt := &ReturnStatement{Token: p.curToken, ReturnValues: []Expression{}}

	p.nextToken() // move past KEMBALIKAN

	// Handle empty return (void)
	// If followed by semicolon, newline (if treated as such), or AKHIR/LAINNYA/EOF
	if p.curTokenIs(lexer.SEMICOLON) {
		return stmt
	}
	// For other terminators that might immediately follow
	if p.curTokenIs(lexer.AKHIR) || p.curTokenIs(lexer.LAINNYA) || p.curTokenIs(lexer.EOF) || p.curTokenIs(lexer.RBRACE) {
		return stmt
	}

	exp := p.parseExpression(LOWEST)
	if exp != nil {
		stmt.ReturnValues = append(stmt.ReturnValues, exp)
	}

	for p.peekTokenIs(lexer.COMMA) {
		p.nextToken()
		p.nextToken()
		exp := p.parseExpression(LOWEST)
		if exp != nil {
			stmt.ReturnValues = append(stmt.ReturnValues, exp)
		}
	}

	if p.peekTokenIs(lexer.SEMICOLON) {
		p.nextToken()
	}

	return stmt
}

func (p *Parser) parseExpressionOrAssignmentStatement() Statement {
	startToken := p.curToken
	exprs := []Expression{p.parseExpression(LOWEST)}

	// Check for multiple LHS (a, b = ...)
	for p.peekTokenIs(lexer.COMMA) {
		p.nextToken()
		p.nextToken()
		exprs = append(exprs, p.parseExpression(LOWEST))
	}

	if p.peekTokenIs(lexer.ASSIGN) {
		p.nextToken() // move to =
		assignToken := p.curToken
		p.nextToken() // move to RHS

		vals := []Expression{p.parseExpression(LOWEST)}
		for p.peekTokenIs(lexer.COMMA) {
			p.nextToken()
			p.nextToken()
			vals = append(vals, p.parseExpression(LOWEST))
		}

		if p.peekTokenIs(lexer.SEMICOLON) {
			p.nextToken()
		}

		return &AssignmentStatement{Token: assignToken, Names: exprs, Values: vals}
	}

	// If we parsed multiple expressions but no assignment, it's invalid (unless we support tuple expressions)
	if len(exprs) > 1 {
		p.addDetailedError(p.curToken, "Comma-separated expressions only allowed in assignment")
		return nil
	}

	stmt := &ExpressionStatement{Token: startToken, Expression: exprs[0]}

	if p.peekTokenIs(lexer.SEMICOLON) {
		p.nextToken()
	}

	return stmt
}

func (p *Parser) parseExpression(precedence int) Expression {
	// Prevent stack overflow from deeply nested expressions
	p.recursionDepth++
	defer func() { p.recursionDepth-- }()

	if p.recursionDepth > MAX_RECURSION_DEPTH {
		p.addDetailedError(p.curToken, "maximum expression nesting depth (%d) exceeded - possible stack overflow", MAX_RECURSION_DEPTH)
		return nil
	}

	prefix := p.prefixParseFns[p.curToken.Type]
	if prefix == nil {
		p.noPrefixParseFnError(p.curToken.Type)
		return nil
	}
	leftExp := prefix()

	for !p.peekTokenIs(lexer.SEMICOLON) && precedence < p.peekPrecedence() {
		infix := p.infixParseFns[p.peekToken.Type]
		if infix == nil {
			return leftExp
		}

		p.nextToken()
		leftExp = infix(leftExp)
	}

	return leftExp
}

func (p *Parser) peekPrecedence() int {
	if p, ok := precedences[p.peekToken.Type]; ok {
		return p
	}
	return LOWEST
}

func (p *Parser) curPrecedence() int {
	if p, ok := precedences[p.curToken.Type]; ok {
		return p
	}
	return LOWEST
}

func (p *Parser) parseIdentifier() Expression {
	return &Identifier{Token: p.curToken, Value: p.curToken.Literal}
}

func (p *Parser) parseIntegerLiteral() Expression {
	lit := &IntegerLiteral{Token: p.curToken}

	value, err := strconv.ParseInt(p.curToken.Literal, 0, 64)
	if err != nil {
		p.addDetailedError(p.curToken, "could not parse %q as integer", p.curToken.Literal)
		return nil
	}

	lit.Value = value
	return lit
}

// parseInterfaceMethod is a simplified version of parseFunctionLiteral for interface method signatures.
// It parses `fungsi Name(params) (returns)` but does not parse a body.
func (p *Parser) parseInterfaceMethod() *FunctionLiteral {
	lit := &FunctionLiteral{Token: p.curToken, Body: nil} // Body is always nil for interfaces

	if !p.expectPeek(lexer.IDENT) {
		p.addDetailedError(p.peekToken, "expected method name after 'fungsi' in interface definition")
		return nil
	}
	lit.Name = p.curToken.Literal

	if !p.expectPeek(lexer.LPAREN) {
		return nil
	}
	lit.Parameters = p.parseFunctionParameters()

	// Parse optional return type(s)
	if p.peekTokenIs(lexer.LPAREN) { // Multiple return types, e.g., (T1, T2)
		p.nextToken() // curToken is (
		p.nextToken() // curToken is Type start
		lit.ReturnTypes = []TypeNode{}

		lit.ReturnTypes = append(lit.ReturnTypes, p.parseType())
		for p.peekTokenIs(lexer.COMMA) {
			p.nextToken()
			p.nextToken()
			lit.ReturnTypes = append(lit.ReturnTypes, p.parseType())
		}

		if !p.expectPeek(lexer.RPAREN) {
			return nil
		}
	} else if p.peekIsType() { // Single return type
		p.nextToken()
		lit.ReturnTypes = []TypeNode{p.parseType()}
		if lit.ReturnTypes[0] == nil {
			return nil
		}
	}

	// Unlike parseFunctionLiteral, we do not expect a block statement.
	// We expect to be at the end of the line/signature.

	return lit
}

func (p *Parser) parseFloatLiteral() Expression {
	lit := &FloatLiteral{Token: p.curToken}

	value, err := strconv.ParseFloat(p.curToken.Literal, 64)
	if err != nil {
		p.addDetailedError(p.curToken, "could not parse %q as float", p.curToken.Literal)
		return nil
	}

	lit.Value = value
	return lit
}

func (p *Parser) parseStringLiteral() Expression {
	// Optimization: If simple string (starts with STRING and no following parts)
	if p.curTokenIs(lexer.STRING) && p.peekToken.Type != lexer.INTERP_START && p.peekToken.Type != lexer.STRING {
		return &StringLiteral{Token: p.curToken, Value: p.curToken.Literal}
	}

	is := &InterpolatedString{Token: p.curToken, Parts: []Expression{}}

	processToken := func() bool {
		if p.curTokenIs(lexer.STRING) {
			is.Parts = append(is.Parts, &StringLiteral{Token: p.curToken, Value: p.curToken.Literal})
			return true
		}
		if p.curTokenIs(lexer.INTERP_START) {
			p.nextToken() // move to expression
			expr := p.parseExpression(LOWEST)
			is.Parts = append(is.Parts, expr)
			if !p.expectPeek(lexer.RBRACE) {
				return false
			}
			return true
		}
		return false
	}

	if !processToken() {
		return nil
	}

	for {
		if p.peekTokenIs(lexer.INTERP_START) || p.peekTokenIs(lexer.STRING) {
			p.nextToken()
			if !processToken() {
				return nil
			}
		} else {
			break
		}
	}
	return is
}

func (p *Parser) parseBoolean() Expression {
	return &BooleanLiteral{Token: p.curToken, Value: p.curTokenIs(lexer.BENAR)}
}

func (p *Parser) parseNull() Expression {
	return &NullLiteral{Token: p.curToken}
}

func (p *Parser) parseArrayLiteral() Expression {
	array := &ArrayLiteral{Token: p.curToken}
	array.Elements = p.parseExpressionList(lexer.RBRACKET)
	return array
}

func (p *Parser) parseExpressionList(end lexer.TokenType) []Expression {
	list := []Expression{}

	if p.peekTokenIs(end) {
		p.nextToken()
		return list
	}

	p.nextToken()
	list = append(list, p.parseExpression(LOWEST))

	for p.peekTokenIs(lexer.COMMA) {
		p.nextToken()
		p.nextToken()
		list = append(list, p.parseExpression(LOWEST))
	}

	if !p.expectPeek(end) {
		return nil
	}

	return list
}

func (p *Parser) parseHashLiteral() Expression {
	hash := &HashLiteral{Token: p.curToken}
	hash.Pairs = make(map[Expression]Expression)

	for !p.peekTokenIs(lexer.RBRACE) {
		p.nextToken()
		key := p.parseExpression(LOWEST)

		if !p.expectPeek(lexer.COLON) {
			return nil
		}

		p.nextToken()
		value := p.parseExpression(LOWEST)

		hash.Pairs[key] = value

		if !p.peekTokenIs(lexer.RBRACE) && !p.expectPeek(lexer.COMMA) {
			return nil
		}
	}

	if !p.expectPeek(lexer.RBRACE) {
		return nil
	}

	return hash
}

func (p *Parser) parseStructLiteral(left Expression) Expression {
	if _, ok := left.(*Identifier); !ok {
		p.addDetailedError(p.curToken, "Struct literal must start with identifier")
		return nil
	}

	lit := &StructLiteral{Token: p.curToken, Name: left}
	lit.Fields = make(map[string]Expression)

	for !p.peekTokenIs(lexer.RBRACE) {
		p.nextToken()
		if !p.curTokenIs(lexer.IDENT) {
			p.addDetailedError(p.curToken, "Expected field name in struct literal")
			return nil
		}
		key := p.curToken.Literal

		if !p.expectPeek(lexer.COLON) {
			return nil
		}

		p.nextToken()
		value := p.parseExpression(LOWEST)
		lit.Fields[key] = value

		if !p.peekTokenIs(lexer.RBRACE) && !p.expectPeek(lexer.COMMA) {
			return nil
		}
	}

	if !p.expectPeek(lexer.RBRACE) {
		return nil
	}

	return lit
}

func (p *Parser) parseIndexExpression(left Expression) Expression {
	exp := &IndexExpression{Token: p.curToken, Left: left}

	p.nextToken()
	exp.Index = p.parseExpression(LOWEST)

	if !p.expectPeek(lexer.RBRACKET) {
		return nil
	}

	return exp
}

func (p *Parser) parsePrefixExpression() Expression {
	expression := &PrefixExpression{
		Token:    p.curToken,
		Operator: p.curToken.Literal,
	}

	p.nextToken()

	expression.Right = p.parseExpression(PREFIX)

	return expression
}

func (p *Parser) parseWhileExpression() Expression {
	expression := &WhileExpression{Token: p.curToken}
	p.nextToken() // eat selama

	expression.Condition = p.parseExpression(LOWEST)

	p.nextToken() // move to block start

	expression.Body = p.parseBlockStatement()

	if p.curTokenIs(lexer.AKHIR) {
		// Consumed by loop? No, loop breaks.
		// We verify it is AKHIR, but do NOT consume it past this node.
	} else {
		p.curError(lexer.AKHIR)
	}

	return expression
}

func (p *Parser) parseInfixExpression(left Expression) Expression {
	expression := &InfixExpression{
		Token:    p.curToken,
		Operator: p.curToken.Literal,
		Left:     left,
	}

	// Strict Whitespace Check
	if isBinaryOp(p.curToken.Type) {
		if !p.curToken.HasLeadingSpace {
			p.addDetailedError(p.curToken, "Binary operator '%s' requires space before it", p.curToken.Literal)
		}
		if !p.peekToken.HasLeadingSpace {
			p.addDetailedError(p.curToken, "Binary operator '%s' requires space after it", p.curToken.Literal)
		}
	}

	precedence := p.curPrecedence()
	p.nextToken()
	expression.Right = p.parseExpression(precedence)

	return expression
}

func isBinaryOp(t lexer.TokenType) bool {
	switch t {
	case lexer.PLUS, lexer.MINUS, lexer.SLASH, lexer.ASTERISK, lexer.PERCENT,
		lexer.EQ, lexer.NOT_EQ, lexer.LT, lexer.GT, lexer.LTE, lexer.GTE,
		lexer.DAN, lexer.ATAU,
		lexer.AND, lexer.OR, lexer.XOR, lexer.LSHIFT, lexer.RSHIFT:
		return true
	}
	return false
}

func (p *Parser) parseGroupedExpression() Expression {
	p.nextToken()

	exp := p.parseExpression(LOWEST)

	if !p.expectPeek(lexer.RPAREN) {
		return nil
	}

	return exp
}

func (p *Parser) parseIfExpression() Expression {
	expression := &IfExpression{Token: p.curToken}

	// curToken is JIKA
	p.nextToken() // eat jika
	expression.Condition = p.parseExpression(LOWEST)

	// Advance to start of block
	p.nextToken()
	expression.Consequence = p.parseBlockStatement()

	for p.curTokenIs(lexer.ATAU_JIKA) {
		clause := ElseIfClause{Token: p.curToken}
		p.nextToken() // eat atau_jika
		clause.Condition = p.parseExpression(LOWEST)
		p.nextToken()
		clause.Consequence = p.parseBlockStatement()
		expression.ElseIfs = append(expression.ElseIfs, clause)
	}

	if p.curTokenIs(lexer.LAINNYA) {
		p.nextToken() // eat lainnya
		expression.Alternative = p.parseBlockStatement()
	}

	if p.curTokenIs(lexer.AKHIR) {
		// Do not consume
	} else {
		p.curError(lexer.AKHIR)
	}

	return expression
}

func (p *Parser) peekIsType() bool {
	t := p.peekToken.Type
	return t == lexer.IDENT || t == lexer.LBRACKET || t == lexer.MAP || t == lexer.ASTERISK || t == lexer.FUNGSI
}

func (p *Parser) parseFunctionLiteral() Expression {
	lit := &FunctionLiteral{Token: p.curToken}
	lit.Doc = p.curComment

	// Check for Method Receiver: fungsi (u User) Name(...)
	// or Anonymous Function: fungsi (a Int) ...
	if p.peekTokenIs(lexer.LPAREN) {
		p.nextToken() // move to LPAREN
		params := p.parseFunctionParameters()

		// Distinguish Method vs Anon Function
		// Method: (params) Name (
		if p.peekTokenIs(lexer.IDENT) && p.peekToken2Is(lexer.LPAREN) {
			// It is a Method
			if len(params) != 1 {
				p.addDetailedError(p.curToken, "Method receiver must have exactly one parameter")
				return nil
			}
			lit.Receiver = params[0]

			p.nextToken() // move to Name
			lit.Name = p.curToken.Literal

			// Parse actual parameters
			if !p.expectPeek(lexer.LPAREN) {
				return nil
			}
			lit.Parameters = p.parseFunctionParameters()
		} else {
			// Anonymous Function
			lit.Parameters = params
		}
	} else if p.peekTokenIs(lexer.IDENT) {
		// Named Function: fungsi Name(...)
		p.nextToken()
		lit.Name = p.curToken.Literal

		if !p.expectPeek(lexer.LPAREN) {
			return nil
		}
		lit.Parameters = p.parseFunctionParameters()
	} else {
		// Expect LPAREN (Anonymous function without params? or malformed)
		if !p.expectPeek(lexer.LPAREN) {
			return nil
		}
		lit.Parameters = p.parseFunctionParameters()
	}

	// Parse optional return type
	if p.peekTokenIs(lexer.LPAREN) {
		p.nextToken() // curToken is (
		p.nextToken() // curToken is Type start
		lit.ReturnTypes = []TypeNode{}

		lit.ReturnTypes = append(lit.ReturnTypes, p.parseType())
		for p.peekTokenIs(lexer.COMMA) {
			p.nextToken()
			p.nextToken()
			lit.ReturnTypes = append(lit.ReturnTypes, p.parseType())
		}

		if !p.expectPeek(lexer.RPAREN) {
			return nil
		}
	} else if p.peekIsType() {
		p.nextToken() // move to type start
		lit.ReturnTypes = []TypeNode{p.parseType()}
		if lit.ReturnTypes[0] == nil {
			return nil
		}
	}

	p.nextToken() // Advance to body start OR native keyword

	if p.curTokenIs(lexer.NATIVE) {
		lit.IsNative = true
		// Optional semicolon
		if p.peekTokenIs(lexer.SEMICOLON) {
			p.nextToken()
		}
		return lit
	}

	lit.Body = p.parseBlockStatement()

	if p.curTokenIs(lexer.AKHIR) {
		// Do not consume
	} else {
		p.curError(lexer.AKHIR)
	}

	return lit
}

func (p *Parser) parseFunctionParameters() []*Parameter {
	identifiers := []*Parameter{}

	if p.peekTokenIs(lexer.RPAREN) {
		p.nextToken()
		return identifiers
	}

	p.nextToken()

	// 1st param
	ident := &Identifier{Token: p.curToken, Value: p.curToken.Literal}
	// type
	p.nextToken()
	typ := p.parseType()
	if typ == nil {
		return nil
	}

	identifiers = append(identifiers, &Parameter{Token: ident.Token, Name: ident, Type: typ})

	for p.peekTokenIs(lexer.COMMA) {
		p.nextToken()
		p.nextToken()

		ident := &Identifier{Token: p.curToken, Value: p.curToken.Literal}
		p.nextToken()
		typ := p.parseType()
		if typ == nil {
			return nil
		}
		identifiers = append(identifiers, &Parameter{Token: ident.Token, Name: ident, Type: typ})
	}

	if !p.expectPeek(lexer.RPAREN) {
		return nil
	}

	return identifiers
}

func (p *Parser) parseCallExpression(function Expression) Expression {
	exp := &CallExpression{Token: p.curToken, Function: function}
	exp.Arguments = p.parseCallArguments()
	return exp
}

func (p *Parser) parseCallArguments() []Expression {
	args := []Expression{}

	if p.peekTokenIs(lexer.RPAREN) {
		p.nextToken()
		return args
	}

	p.nextToken()
	args = append(args, p.parseExpression(LOWEST))

	for p.peekTokenIs(lexer.COMMA) {
		p.nextToken()
		p.nextToken()
		args = append(args, p.parseExpression(LOWEST))
	}

	if !p.expectPeek(lexer.RPAREN) {
		return nil
	}

	return args
}

func (p *Parser) parseBlockStatement() *BlockStatement {
	block := &BlockStatement{Token: p.curToken}
	block.Statements = []Statement{}

	for !p.curTokenIs(lexer.AKHIR) && !p.curTokenIs(lexer.LAINNYA) && !p.curTokenIs(lexer.ATAU_JIKA) && !p.curTokenIs(lexer.KASUS) && !p.curTokenIs(lexer.EOF) {
		stmt := p.parseStatement()
		if stmt != nil {
			block.Statements = append(block.Statements, stmt)
		}
		p.nextToken()
	}

	return block
}

func (p *Parser) curTokenIs(t lexer.TokenType) bool {
	return p.curToken.Type == t
}

func (p *Parser) peekTokenIs(t lexer.TokenType) bool {
	return p.peekToken.Type == t
}

func (p *Parser) expectPeek(t lexer.TokenType) bool {
	if p.peekTokenIs(t) {
		p.nextToken()
		return true
	}
	p.peekError(t)
	return false
}

func (p *Parser) noPrefixParseFnError(t lexer.TokenType) {
	p.addDetailedError(p.curToken, "no prefix parse function for %s found", t)
}

func (p *Parser) parseDotExpression(left Expression) Expression {
	exp := &MemberExpression{
		Token:  p.curToken, // DOT token
		Object: left,
	}

	if !p.expectPeek(lexer.IDENT) {
		return nil
	}

	exp.Member = &Identifier{
		Token: p.curToken,
		Value: p.curToken.Literal,
	}

	return exp
}
