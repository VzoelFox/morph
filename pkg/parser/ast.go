package parser

import (
	"bytes"
	"strings"

	"github.com/VzoelFox/morph/pkg/lexer"
)

// The base Node interface
type Node interface {
	TokenLiteral() string
	String() string
}

type Statement interface {
	Node
	statementNode()
}

type Expression interface {
	Node
	expressionNode()
}

// TypeNode represents a type annotation in the AST
type TypeNode interface {
	Node
	typeNode()
}

type Program struct {
	Statements []Statement
}

func (p *Program) TokenLiteral() string {
	if len(p.Statements) > 0 {
		return p.Statements[0].TokenLiteral()
	}
	return ""
}

func (p *Program) String() string {
	var out bytes.Buffer
	for _, s := range p.Statements {
		out.WriteString(s.String())
	}
	return out.String()
}

// --- Type Nodes ---

type SimpleType struct {
	Token lexer.Token
	Name  string
}

func (st *SimpleType) typeNode()            {}
func (st *SimpleType) TokenLiteral() string { return st.Token.Literal }
func (st *SimpleType) String() string       { return st.Name }

type QualifiedType struct {
	Token   lexer.Token // The first identifier
	Package *Identifier
	Name    *Identifier
}

func (qt *QualifiedType) typeNode()            {}
func (qt *QualifiedType) TokenLiteral() string { return qt.Token.Literal }
func (qt *QualifiedType) String() string {
	return qt.Package.String() + "." + qt.Name.String()
}

type ArrayType struct {
	Token   lexer.Token // '['
	Element TypeNode
}

func (at *ArrayType) typeNode()            {}
func (at *ArrayType) TokenLiteral() string { return at.Token.Literal }
func (at *ArrayType) String() string       { return "[]" + at.Element.String() }

type PointerType struct {
	Token   lexer.Token // '*'
	Element TypeNode
}

func (pt *PointerType) typeNode()            {}
func (pt *PointerType) TokenLiteral() string { return pt.Token.Literal }
func (pt *PointerType) String() string       { return "*" + pt.Element.String() }

type MapType struct {
	Token lexer.Token // 'map'
	Key   TypeNode
	Value TypeNode
}

func (mt *MapType) typeNode()            {}
func (mt *MapType) TokenLiteral() string { return mt.Token.Literal }
func (mt *MapType) String() string {
	return "map[" + mt.Key.String() + "]" + mt.Value.String()
}

// --- Statements ---

type VarStatement struct {
	Token   lexer.Token // var or tetapan
	Names   []*Identifier
	Type    TypeNode // Optional
	Values  []Expression
	IsConst bool
}

func (vs *VarStatement) statementNode()       {}
func (vs *VarStatement) TokenLiteral() string { return vs.Token.Literal }
func (vs *VarStatement) String() string {
	var out bytes.Buffer
	out.WriteString(vs.TokenLiteral() + " ")

	names := []string{}
	for _, n := range vs.Names {
		names = append(names, n.String())
	}
	out.WriteString(strings.Join(names, ", "))

	if vs.Type != nil {
		out.WriteString(" " + vs.Type.String())
	}

	if len(vs.Values) > 0 {
		out.WriteString(" = ")
		vals := []string{}
		for _, v := range vs.Values {
			vals = append(vals, v.String())
		}
		out.WriteString(strings.Join(vals, ", "))
	}

	out.WriteString(";")
	return out.String()
}

type ExpressionStatement struct {
	Token      lexer.Token
	Expression Expression
}

func (es *ExpressionStatement) statementNode()       {}
func (es *ExpressionStatement) TokenLiteral() string { return es.Token.Literal }
func (es *ExpressionStatement) String() string {
	if es.Expression != nil {
		return es.Expression.String()
	}
	return ""
}

type Identifier struct {
	Token lexer.Token
	Value string
}

func (i *Identifier) expressionNode()      {}
func (i *Identifier) TokenLiteral() string { return i.Token.Literal }
func (i *Identifier) String() string       { return i.Value }

type IntegerLiteral struct {
	Token lexer.Token
	Value int64
}

func (il *IntegerLiteral) expressionNode()      {}
func (il *IntegerLiteral) TokenLiteral() string { return il.Token.Literal }
func (il *IntegerLiteral) String() string       { return il.Token.Literal }

type FloatLiteral struct {
	Token lexer.Token
	Value float64
}

func (fl *FloatLiteral) expressionNode()      {}
func (fl *FloatLiteral) TokenLiteral() string { return fl.Token.Literal }
func (fl *FloatLiteral) String() string       { return fl.Token.Literal }

type StringLiteral struct {
	Token lexer.Token
	Value string
}

func (sl *StringLiteral) expressionNode()      {}
func (sl *StringLiteral) TokenLiteral() string { return sl.Token.Literal }
func (sl *StringLiteral) String() string       { return sl.Token.Literal }

type InterpolatedString struct {
	Token lexer.Token // The first part
	Parts []Expression
}

func (is *InterpolatedString) expressionNode()      {}
func (is *InterpolatedString) TokenLiteral() string { return is.Token.Literal }
func (is *InterpolatedString) String() string {
	var out bytes.Buffer
	out.WriteString("\"")
	for _, p := range is.Parts {
		if sl, ok := p.(*StringLiteral); ok {
			out.WriteString(sl.Value)
		} else {
			out.WriteString("#{")
			out.WriteString(p.String())
			out.WriteString("}")
		}
	}
	out.WriteString("\"")
	return out.String()
}

type BooleanLiteral struct {
	Token lexer.Token
	Value bool
}

func (b *BooleanLiteral) expressionNode()      {}
func (b *BooleanLiteral) TokenLiteral() string { return b.Token.Literal }
func (b *BooleanLiteral) String() string       { return b.Token.Literal }

type NullLiteral struct {
	Token lexer.Token
}

func (n *NullLiteral) expressionNode()      {}
func (n *NullLiteral) TokenLiteral() string { return n.Token.Literal }
func (n *NullLiteral) String() string       { return n.Token.Literal }

type CharLiteral struct {
	Token lexer.Token
	Value int64
}

func (cl *CharLiteral) expressionNode()      {}
func (cl *CharLiteral) TokenLiteral() string { return cl.Token.Literal }
func (cl *CharLiteral) String() string       { return cl.Token.Literal }

type ArrayLiteral struct {
	Token    lexer.Token // '['
	Elements []Expression
}

func (al *ArrayLiteral) expressionNode()      {}
func (al *ArrayLiteral) TokenLiteral() string { return al.Token.Literal }
func (al *ArrayLiteral) String() string {
	var out bytes.Buffer
	elements := []string{}
	for _, el := range al.Elements {
		elements = append(elements, el.String())
	}
	out.WriteString("[")
	out.WriteString(strings.Join(elements, ", "))
	out.WriteString("]")
	return out.String()
}

type HashLiteral struct {
	Token lexer.Token // '{'
	Pairs map[Expression]Expression
}

func (hl *HashLiteral) expressionNode()      {}
func (hl *HashLiteral) TokenLiteral() string { return hl.Token.Literal }
func (hl *HashLiteral) String() string {
	var out bytes.Buffer
	pairs := []string{}
	for key, value := range hl.Pairs {
		pairs = append(pairs, key.String()+":"+value.String())
	}
	out.WriteString("{")
	out.WriteString(strings.Join(pairs, ", "))
	out.WriteString("}")
	return out.String()
}

type StructLiteral struct {
	Token  lexer.Token // '{'
	Name   Expression  // Identifier
	Fields map[string]Expression
}

func (sl *StructLiteral) expressionNode()      {}
func (sl *StructLiteral) TokenLiteral() string { return sl.Token.Literal }
func (sl *StructLiteral) String() string {
	var out bytes.Buffer
	out.WriteString(sl.Name.String())
	out.WriteString("{")
	fields := []string{}
	for key, value := range sl.Fields {
		fields = append(fields, key+": "+value.String())
	}
	out.WriteString(strings.Join(fields, ", "))
	out.WriteString("}")
	return out.String()
}

type IndexExpression struct {
	Token lexer.Token // The [ token
	Left  Expression
	Index Expression
}

func (ie *IndexExpression) expressionNode()      {}
func (ie *IndexExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *IndexExpression) String() string {
	var out bytes.Buffer
	out.WriteString("(")
	out.WriteString(ie.Left.String())
	out.WriteString("[")
	out.WriteString(ie.Index.String())
	out.WriteString("])")
	return out.String()
}

type MemberExpression struct {
	Token  lexer.Token // The . token
	Object Expression
	Member *Identifier
}

func (me *MemberExpression) expressionNode()      {}
func (me *MemberExpression) TokenLiteral() string { return me.Token.Literal }
func (me *MemberExpression) String() string {
	var out bytes.Buffer
	out.WriteString(me.Object.String())
	out.WriteString(".")
	out.WriteString(me.Member.String())
	return out.String()
}

type PrefixExpression struct {
	Token    lexer.Token
	Operator string
	Right    Expression
}

func (pe *PrefixExpression) expressionNode()      {}
func (pe *PrefixExpression) TokenLiteral() string { return pe.Token.Literal }
func (pe *PrefixExpression) String() string {
	var out bytes.Buffer
	out.WriteString("(")
	out.WriteString(pe.Operator)
	out.WriteString(pe.Right.String())
	out.WriteString(")")
	return out.String()
}

type InfixExpression struct {
	Token    lexer.Token
	Left     Expression
	Operator string
	Right    Expression
}

func (ie *InfixExpression) expressionNode()      {}
func (ie *InfixExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *InfixExpression) String() string {
	var out bytes.Buffer
	out.WriteString("(")
	out.WriteString(ie.Left.String())
	out.WriteString(" " + ie.Operator + " ")
	out.WriteString(ie.Right.String())
	out.WriteString(")")
	return out.String()
}

type BlockStatement struct {
	Token      lexer.Token
	Statements []Statement
}

func (bs *BlockStatement) statementNode()       {}
func (bs *BlockStatement) TokenLiteral() string { return bs.Token.Literal }
func (bs *BlockStatement) String() string {
	var out bytes.Buffer
	for _, s := range bs.Statements {
		out.WriteString(s.String())
	}
	return out.String()
}

type IfExpression struct {
	Token       lexer.Token
	Condition   Expression
	Consequence *BlockStatement
	ElseIfs     []ElseIfClause
	Alternative *BlockStatement
}

type ElseIfClause struct {
	Token       lexer.Token
	Condition   Expression
	Consequence *BlockStatement
}

func (ie *IfExpression) expressionNode()      {}
func (ie *IfExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *IfExpression) String() string {
	var out bytes.Buffer
	out.WriteString("jika ")
	out.WriteString(ie.Condition.String())
	out.WriteString(" ")
	out.WriteString(ie.Consequence.String())
	for _, clause := range ie.ElseIfs {
		out.WriteString(" atau_jika ")
		out.WriteString(clause.Condition.String())
		out.WriteString(" ")
		out.WriteString(clause.Consequence.String())
	}
	if ie.Alternative != nil {
		out.WriteString(" lainnya ")
		out.WriteString(ie.Alternative.String())
	}
	out.WriteString(" akhir")
	return out.String()
}

type WhileExpression struct {
	Token     lexer.Token
	Condition Expression
	Body      *BlockStatement
}

func (we *WhileExpression) expressionNode()      {}
func (we *WhileExpression) TokenLiteral() string { return we.Token.Literal }
func (we *WhileExpression) String() string {
	var out bytes.Buffer
	out.WriteString("selama ")
	out.WriteString(we.Condition.String())
	out.WriteString(" ")
	out.WriteString(we.Body.String())
	out.WriteString(" akhir")
	return out.String()
}

type CaseClause struct {
	Token  lexer.Token // kasus
	Values []Expression
	Body   *BlockStatement
}

func (cc *CaseClause) String() string {
	var out bytes.Buffer
	out.WriteString("kasus ")
	vals := []string{}
	for _, v := range cc.Values {
		vals = append(vals, v.String())
	}
	out.WriteString(strings.Join(vals, ", "))
	out.WriteString(": ")
	out.WriteString(cc.Body.String())
	return out.String()
}

type SwitchStatement struct {
	Token     lexer.Token // pilih
	Condition Expression
	Cases     []*CaseClause
	Default   *BlockStatement // lainnya
}

func (ss *SwitchStatement) statementNode()       {}
func (ss *SwitchStatement) TokenLiteral() string { return ss.Token.Literal }
func (ss *SwitchStatement) String() string {
	var out bytes.Buffer
	out.WriteString("pilih ")
	out.WriteString(ss.Condition.String())
	out.WriteString(" ")
	for _, c := range ss.Cases {
		out.WriteString(c.String())
	}
	if ss.Default != nil {
		out.WriteString("lainnya: ")
		out.WriteString(ss.Default.String())
	}
	out.WriteString("akhir")
	return out.String()
}

type Parameter struct {
	Token lexer.Token
	Name  *Identifier
	Type  TypeNode
}

func (p *Parameter) String() string {
	if p.Type != nil {
		return p.Name.String() + " " + p.Type.String()
	}
	return p.Name.String()
}

type FunctionLiteral struct {
	Token       lexer.Token
	Name        string
	Receiver    *Parameter // Optional receiver for methods
	Parameters  []*Parameter
	ReturnTypes []TypeNode
	Body        *BlockStatement
	IsNative    bool
	Doc         string
}

func (fl *FunctionLiteral) expressionNode()      {}
func (fl *FunctionLiteral) TokenLiteral() string { return fl.Token.Literal }
func (fl *FunctionLiteral) String() string {
	var out bytes.Buffer
	out.WriteString(fl.TokenLiteral())

	if fl.Receiver != nil {
		out.WriteString(" (")
		out.WriteString(fl.Receiver.String())
		out.WriteString(")")
	}

	if fl.Name != "" {
		out.WriteString(" ")
		out.WriteString(fl.Name)
	}
	out.WriteString("(")
	for i, p := range fl.Parameters {
		out.WriteString(p.String())
		if i < len(fl.Parameters)-1 {
			out.WriteString(", ")
		}
	}
	out.WriteString(") ")

	if len(fl.ReturnTypes) > 0 {
		if len(fl.ReturnTypes) > 1 {
			out.WriteString("(")
		}
		for i, rt := range fl.ReturnTypes {
			if i > 0 {
				out.WriteString(", ")
			}
			out.WriteString(rt.String())
		}
		if len(fl.ReturnTypes) > 1 {
			out.WriteString(")")
		}
		out.WriteString(" ")
	}

	if fl.IsNative {
		out.WriteString("native")
	} else {
		out.WriteString(fl.Body.String())
		out.WriteString(" akhir")
	}
	return out.String()
}

type CallExpression struct {
	Token     lexer.Token
	Function  Expression
	Arguments []Expression
}

func (ce *CallExpression) expressionNode()      {}
func (ce *CallExpression) TokenLiteral() string { return ce.Token.Literal }
func (ce *CallExpression) String() string {
	var out bytes.Buffer
	out.WriteString(ce.Function.String())
	out.WriteString("(")
	for i, a := range ce.Arguments {
		out.WriteString(a.String())
		if i < len(ce.Arguments)-1 {
			out.WriteString(", ")
		}
	}
	out.WriteString(")")
	return out.String()
}

type ReturnStatement struct {
	Token        lexer.Token
	ReturnValues []Expression
}

func (rs *ReturnStatement) statementNode()       {}
func (rs *ReturnStatement) TokenLiteral() string { return rs.Token.Literal }
func (rs *ReturnStatement) String() string {
	var out bytes.Buffer
	out.WriteString(rs.TokenLiteral() + " ")

	vals := []string{}
	for _, v := range rs.ReturnValues {
		vals = append(vals, v.String())
	}
	out.WriteString(strings.Join(vals, ", "))

	out.WriteString(";")
	return out.String()
}

type StructField struct {
	Token lexer.Token
	Name  string
	Type  TypeNode // changed from string
}

func (sf *StructField) String() string { return sf.Name + " " + sf.Type.String() }

type StructStatement struct {
	Token  lexer.Token // The 'struktur' token
	Name   *Identifier
	Fields []*StructField
	Doc    string
}

func (ss *StructStatement) statementNode()       {}
func (ss *StructStatement) TokenLiteral() string { return ss.Token.Literal }
func (ss *StructStatement) String() string {
	var out bytes.Buffer
	out.WriteString("struktur ")
	out.WriteString(ss.Name.String())
	out.WriteString("\n")
	for _, f := range ss.Fields {
		out.WriteString("  " + f.String() + "\n")
	}
	out.WriteString("akhir")
	return out.String()
}

// InterfaceStatement represents the definition of an interface.
// e.g., antarmuka Pembicara fungsi Bicara() string akhir
type InterfaceStatement struct {
	Token   lexer.Token // The 'antarmuka' token
	Name    *Identifier
	Methods []*FunctionLiteral // Method signatures (Body will be nil)
}

func (is *InterfaceStatement) statementNode()       {}
func (is *InterfaceStatement) TokenLiteral() string { return is.Token.Literal }
func (is *InterfaceStatement) String() string {
	var out bytes.Buffer
	out.WriteString("antarmuka ")
	out.WriteString(is.Name.String())
	out.WriteString("\n")
	for _, m := range is.Methods {
		// Custom string for method signature without body
		var methodStr bytes.Buffer
		methodStr.WriteString("  fungsi ")
		methodStr.WriteString(m.Name)
		methodStr.WriteString("(")
		params := []string{}
		for _, p := range m.Parameters {
			params = append(params, p.String())
		}
		methodStr.WriteString(strings.Join(params, ", "))
		methodStr.WriteString(")")
		if len(m.ReturnTypes) > 0 {
			methodStr.WriteString(" ")
			returns := []string{}
			for _, rt := range m.ReturnTypes {
				returns = append(returns, rt.String())
			}
			methodStr.WriteString(strings.Join(returns, ", "))
		}
		methodStr.WriteString("\n")
		out.WriteString(methodStr.String())
	}
	out.WriteString("akhir")
	return out.String()
}

type BreakStatement struct {
	Token lexer.Token
}

func (bs *BreakStatement) statementNode()       {}
func (bs *BreakStatement) TokenLiteral() string { return bs.Token.Literal }
func (bs *BreakStatement) String() string       { return bs.Token.Literal + ";" }

type ContinueStatement struct {
	Token lexer.Token
}

func (cs *ContinueStatement) statementNode()       {}
func (cs *ContinueStatement) TokenLiteral() string { return cs.Token.Literal }
func (cs *ContinueStatement) String() string       { return cs.Token.Literal + ";" }

type ImportStatement struct {
	Token       lexer.Token // The 'ambil' or 'dari' token
	Path        string      // The file path
	Identifiers []string    // Imported identifiers (if 'dari')
}

func (is *ImportStatement) statementNode()       {}
func (is *ImportStatement) TokenLiteral() string { return is.Token.Literal }
func (is *ImportStatement) String() string {
	var out bytes.Buffer
	if len(is.Identifiers) > 0 {
		out.WriteString("dari \"")
		out.WriteString(is.Path)
		out.WriteString("\" ambil ")
		out.WriteString(strings.Join(is.Identifiers, ", "))
	} else {
		out.WriteString("ambil \"")
		out.WriteString(is.Path)
		out.WriteString("\"")
	}
	out.WriteString(";")
	return out.String()
}

type AssignmentStatement struct {
	Token  lexer.Token
	Names  []Expression
	Values []Expression
}

func (as *AssignmentStatement) statementNode()       {}
func (as *AssignmentStatement) TokenLiteral() string { return as.Token.Literal }
func (as *AssignmentStatement) String() string {
	var out bytes.Buffer

	names := []string{}
	for _, n := range as.Names {
		names = append(names, n.String())
	}
	out.WriteString(strings.Join(names, ", "))

	out.WriteString(" = ")

	vals := []string{}
	for _, v := range as.Values {
		vals = append(vals, v.String())
	}
	out.WriteString(strings.Join(vals, ", "))

	out.WriteString(";")
	return out.String()
}

type FunctionType struct {
	Token       lexer.Token // The 'fungsi' token
	Parameters  []TypeNode
	ReturnTypes []TypeNode
}

func (ft *FunctionType) typeNode()            {}
func (ft *FunctionType) TokenLiteral() string { return ft.Token.Literal }
func (ft *FunctionType) String() string {
	var out bytes.Buffer
	out.WriteString("fungsi")
	out.WriteString("(")
	params := []string{}
	for _, p := range ft.Parameters {
		params = append(params, p.String())
	}
	out.WriteString(strings.Join(params, ", "))
	out.WriteString(")")
	if len(ft.ReturnTypes) > 0 {
		out.WriteString(" ")
		if len(ft.ReturnTypes) > 1 {
			out.WriteString("(")
		}
		returns := []string{}
		for _, rt := range ft.ReturnTypes {
			returns = append(returns, rt.String())
		}
		out.WriteString(strings.Join(returns, ", "))
		if len(ft.ReturnTypes) > 1 {
			out.WriteString(")")
		}
	}
	return out.String()
}
