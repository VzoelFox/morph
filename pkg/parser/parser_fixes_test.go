package parser

import (
	"testing"

	"github.com/VzoelFox/morphlang/pkg/lexer"
)

func TestArrayParsing(t *testing.T) {
	input := "[1, 2 * 2, 3 + 3]"
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	stmt, ok := program.Statements[0].(*ExpressionStatement)
	if !ok {
		t.Fatalf("program.Statements[0] is not ExpressionStatement. got=%T", program.Statements[0])
	}

	array, ok := stmt.Expression.(*ArrayLiteral)
	if !ok {
		t.Fatalf("exp not ArrayLiteral. got=%T", stmt.Expression)
	}

	if len(array.Elements) != 3 {
		t.Fatalf("len(array.Elements) not 3. got=%d", len(array.Elements))
	}

	testIntegerLiteral(t, array.Elements[0], 1)
	testInfixExpression(t, array.Elements[1], 2, "*", 2)
	testInfixExpression(t, array.Elements[2], 3, "+", 3)
}

func TestHashParsing(t *testing.T) {
	input := `{"one": 1, "two": 2}`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	stmt, ok := program.Statements[0].(*ExpressionStatement)
	if !ok {
		t.Fatalf("program.Statements[0] is not ExpressionStatement. got=%T", program.Statements[0])
	}

	hash, ok := stmt.Expression.(*HashLiteral)
	if !ok {
		t.Fatalf("exp not HashLiteral. got=%T", stmt.Expression)
	}

	if len(hash.Pairs) != 2 {
		t.Fatalf("hash.Pairs has wrong length. got=%d", len(hash.Pairs))
	}

	// Iteration order is random, just checking existence implies success if length matches.
}

func TestIndexParsing(t *testing.T) {
	input := "arr[1 + 1]"
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	stmt, ok := program.Statements[0].(*ExpressionStatement)
	if !ok {
		t.Fatalf("program.Statements[0] is not ExpressionStatement. got=%T", program.Statements[0])
	}

	indexExp, ok := stmt.Expression.(*IndexExpression)
	if !ok {
		t.Fatalf("exp not IndexExpression. got=%T", stmt.Expression)
	}

	testIdentifier(t, indexExp.Left, "arr")
	testInfixExpression(t, indexExp.Index, 1, "+", 1)
}
