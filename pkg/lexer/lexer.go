package lexer

type Lexer struct {
	input        string
	position     int
	readPosition int
	ch           byte
	line         int
	column       int

	states      []int
	braceCounts []int
}

const (
	STATE_CODE   = 0
	STATE_STRING = 1
)

func New(input string) *Lexer {
	l := &Lexer{
		input:       input,
		line:        1,
		column:      0,
		states:      []int{STATE_CODE},
		braceCounts: []int{0},
	}
	l.readChar()
	return l
}

func (l *Lexer) currentState() int {
	if len(l.states) == 0 {
		return STATE_CODE
	}
	return l.states[len(l.states)-1]
}

func (l *Lexer) pushState(s int) {
	l.states = append(l.states, s)
	if s == STATE_CODE {
		l.braceCounts = append(l.braceCounts, 0)
	}
}

func (l *Lexer) popState() {
	if len(l.states) == 0 {
		return
	}
	s := l.states[len(l.states)-1]
	l.states = l.states[:len(l.states)-1]
	if s == STATE_CODE {
		if len(l.braceCounts) > 0 {
			l.braceCounts = l.braceCounts[:len(l.braceCounts)-1]
		}
	}
}

func (l *Lexer) currentBraceCount() int {
	if len(l.braceCounts) == 0 {
		return 0
	}
	return l.braceCounts[len(l.braceCounts)-1]
}

func (l *Lexer) incrementBraceCount() {
	if len(l.braceCounts) > 0 {
		l.braceCounts[len(l.braceCounts)-1]++
	}
}

func (l *Lexer) decrementBraceCount() {
	if len(l.braceCounts) > 0 {
		l.braceCounts[len(l.braceCounts)-1]--
	}
}

func (l *Lexer) readChar() {
	if l.readPosition >= len(l.input) {
		l.ch = 0
	} else {
		l.ch = l.input[l.readPosition]
	}
	l.position = l.readPosition
	l.readPosition += 1
	l.column += 1
}

func (l *Lexer) peekChar() byte {
	if l.readPosition >= len(l.input) {
		return 0
	}
	return l.input[l.readPosition]
}

func (l *Lexer) NextToken() Token {
	if l.currentState() == STATE_STRING {
		// Continuation of string (e.g. after interpolation) usually has no leading space
		// relative to the code stream, as it is inside quotes.
		return l.readStringToken(false)
	}
	return l.readCodeToken()
}

func (l *Lexer) readCodeToken() Token {
	var tok Token

	startPos := l.position
	l.skipWhitespace()
	hasLeadingSpace := l.position > startPos

	tokLine := l.line
	tokCol := l.column

	switch l.ch {
	case '=':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			literal := string(ch) + string(l.ch)
			tok = Token{Type: EQ, Literal: literal}
		} else {
			tok = newToken(ASSIGN, l.ch)
		}
	case '+':
		tok = newToken(PLUS, l.ch)
	case '-':
		tok = newToken(MINUS, l.ch)
	case '!':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			literal := string(ch) + string(l.ch)
			tok = Token{Type: NOT_EQ, Literal: literal}
		} else {
			tok = newToken(BANG, l.ch)
		}
	case '/':
		tok = newToken(SLASH, l.ch)
	case '*':
		tok = newToken(ASTERISK, l.ch)
	case '&':
		tok = newToken(AND, l.ch)
	case '|':
		tok = newToken(OR, l.ch)
	case '^':
		tok = newToken(XOR, l.ch)
	case '~':
		tok = newToken(TILDE, l.ch)
	case '<':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			literal := string(ch) + string(l.ch)
			tok = Token{Type: LTE, Literal: literal}
		} else if l.peekChar() == '<' {
			ch := l.ch
			l.readChar()
			literal := string(ch) + string(l.ch)
			tok = Token{Type: LSHIFT, Literal: literal}
		} else {
			tok = newToken(LT, l.ch)
		}
	case '>':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			literal := string(ch) + string(l.ch)
			tok = Token{Type: GTE, Literal: literal}
		} else if l.peekChar() == '>' {
			ch := l.ch
			l.readChar()
			literal := string(ch) + string(l.ch)
			tok = Token{Type: RSHIFT, Literal: literal}
		} else {
			tok = newToken(GT, l.ch)
		}
	case ';':
		tok = newToken(SEMICOLON, l.ch)
	case ':':
		tok = newToken(COLON, l.ch)
	case ',':
		tok = newToken(COMMA, l.ch)
	case '.':
		tok = newToken(DOT, l.ch)
	case '(':
		tok = newToken(LPAREN, l.ch)
	case ')':
		tok = newToken(RPAREN, l.ch)
	case '{':
		l.incrementBraceCount()
		tok = newToken(LBRACE, l.ch)
	case '}':
		if len(l.braceCounts) > 1 && l.currentBraceCount() == 0 {
			l.popState()
			tok = newToken(RBRACE, l.ch)
		} else {
			l.decrementBraceCount()
			tok = newToken(RBRACE, l.ch)
		}
	case '[':
		tok = newToken(LBRACKET, l.ch)
	case ']':
		tok = newToken(RBRACKET, l.ch)
	case '#':
		tok = Token{Type: COMMENT, Literal: l.readComment(), Line: tokLine, Column: tokCol, HasLeadingSpace: hasLeadingSpace}
		return tok
	case '"':
		// Optimization for empty string
		if l.peekChar() == '"' {
			tok = Token{Type: STRING, Literal: "", Line: tokLine, Column: tokCol, HasLeadingSpace: hasLeadingSpace}
			l.readChar() // consume opening "
			// consume closing " happens at end of function if logic flowed there, but here we return early?
			// Wait, previous logic was: l.readChar(); return ...
			// But readStringToken expects to read content.
			// Let's keep logic simple: push state, delegate to readStringToken.
			// Optimizations might be tricky with HasLeadingSpace.
			// Let's remove optimization for clarity/safety or fix it.
			// If empty string: ""
			l.readChar() // eat opening "
			// check closing
			if l.ch == '"' {
				tok = Token{Type: STRING, Literal: "", Line: tokLine, Column: tokCol, HasLeadingSpace: hasLeadingSpace}
				l.readChar() // eat closing "
				return tok
			}
			// Not empty immediately (or logic above was just optimization).
			// Let's stick to standard path.
			// Rewind? No.
			// Just use readStringToken logic.
			l.pushState(STATE_STRING)
			// l.readChar() was done (consumed opening ").
			// But readStringToken expects to start reading content.
			return l.readStringToken(hasLeadingSpace)
		} else {
			l.pushState(STATE_STRING)
			l.readChar() // consume opening "
			return l.readStringToken(hasLeadingSpace)
		}
	case 0:
		tok.Literal = ""
		tok.Type = EOF
	default:
		if isLetter(l.ch) {
			tok.Literal = l.readIdentifier()
			tok.Type = LookupIdent(tok.Literal)
			tok.Line = tokLine
			tok.Column = tokCol
			tok.HasLeadingSpace = hasLeadingSpace
			return tok
		} else if isDigit(l.ch) {
			tok.Literal, tok.Type = l.readNumber()
			tok.Line = tokLine
			tok.Column = tokCol
			tok.HasLeadingSpace = hasLeadingSpace
			return tok
		} else {
			tok = newToken(ILLEGAL, l.ch)
		}
	}

	tok.Line = tokLine
	tok.Column = tokCol
	tok.HasLeadingSpace = hasLeadingSpace

	l.readChar()
	return tok
}

func (l *Lexer) readStringToken(hasLeadingSpace bool) Token {
	tokLine := l.line
	tokCol := l.column

	if l.ch == '"' {
		l.popState()
		l.readChar() // consume closing quote
		return l.NextToken()
	}

	if l.ch == '#' && l.peekChar() == '{' {
		l.pushState(STATE_CODE)
		// Interpolation start. Does it have leading space? No, inside string.
		tok := Token{Type: INTERP_START, Literal: "#{", Line: tokLine, Column: tokCol, HasLeadingSpace: false}
		l.readChar() // consume #
		l.readChar() // consume {
		return tok
	}

	content := l.readStringContent()
	return Token{
		Type:            STRING,
		Literal:         content,
		Line:            tokLine,
		Column:          tokCol,
		HasLeadingSpace: hasLeadingSpace,
	}
}

func newToken(tokenType TokenType, ch byte) Token {
	return Token{Type: tokenType, Literal: string(ch)}
}

func (l *Lexer) readIdentifier() string {
	position := l.position
	for isLetter(l.ch) || l.ch == '_' || isDigit(l.ch) {
		l.readChar()
	}
	return l.input[position:l.position]
}

func isLetter(ch byte) bool {
	return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_'
}

func (l *Lexer) readNumber() (string, TokenType) {
	position := l.position
	for isDigit(l.ch) {
		l.readChar()
	}

	if l.ch == '.' && isDigit(l.peekChar()) {
		l.readChar() // consume dot
		for isDigit(l.ch) {
			l.readChar()
		}
		return l.input[position:l.position], FLOAT
	}

	return l.input[position:l.position], INT
}

func isDigit(ch byte) bool {
	return '0' <= ch && ch <= '9'
}

func (l *Lexer) readStringContent() string {
	var out string
	for {
		if l.ch == '"' || l.ch == 0 {
			break
		}
		if l.ch == '#' && l.peekChar() == '{' {
			break
		}

		if l.ch == '\\' {
			l.readChar()
			switch l.ch {
			case 'n':
				out += "\n"
			case 't':
				out += "\t"
			case '"':
				out += "\""
			case '\\':
				out += "\\"
			case 'r':
				out += "\r"
			default:
				out += string(l.ch)
			}
		} else {
			out += string(l.ch)
		}
		l.readChar()
	}
	return out
}

func (l *Lexer) skipWhitespace() {
	for l.ch == ' ' || l.ch == '\t' || l.ch == '\n' || l.ch == '\r' {
		if l.ch == '\n' {
			l.line += 1
			l.column = 0
		}
		l.readChar()
	}
}

func (l *Lexer) readComment() string {
	l.readChar() // consume '#'
	if l.ch == ' ' {
		l.readChar()
	}
	position := l.position
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
	return l.input[position:l.position]
}
