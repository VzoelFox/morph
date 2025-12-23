package checker

import (
	"testing"
)

func TestInterfaceType(t *testing.T) {
	// Definisikan beberapa tipe fungsi untuk digunakan dalam metode interface
	method1 := &FunctionType{
		Parameters:  []Type{IntType},
		ReturnTypes: []Type{StringType},
	}
	method2 := &FunctionType{
		Parameters:  []Type{},
		ReturnTypes: []Type{BoolType},
	}

	// Buat beberapa instance InterfaceType
	iface1 := &InterfaceType{
		Name: "Writer",
		Methods: map[string]*FunctionType{
			"Write": method1,
		},
	}
	iface1_clone := &InterfaceType{
		Name: "Writer",
		Methods: map[string]*FunctionType{
			"Write": method1,
		},
	}
	iface2 := &InterfaceType{
		Name: "Reader",
		Methods: map[string]*FunctionType{
			"Read": method2,
		},
	}

	t.Run("Kind", func(t *testing.T) {
		if iface1.Kind() != KindInterface {
			t.Errorf("expected kind %v, got %v", KindInterface, iface1.Kind())
		}
	})

	t.Run("string", func(t *testing.T) {
		if iface1.String() != "Writer" {
			t.Errorf("expected string 'Writer', got '%s'", iface1.String())
		}
		if iface2.String() != "Reader" {
			t.Errorf("expected string 'Reader', got '%s'", iface2.String())
		}
	})

	t.Run("Equals", func(t *testing.T) {
		// Test kesetaraan nominal (berdasarkan nama)
		if !iface1.Equals(iface1_clone) {
			t.Errorf("expected iface1 to be equal to iface1_clone")
		}
		if iface1.Equals(iface2) {
			t.Errorf("expected iface1 to not be equal to iface2")
		}

		// Test terhadap tipe lain
		if iface1.Equals(IntType) {
			t.Errorf("expected iface1 not to be equal to IntType")
		}

		// Test terhadap null
		if iface1.Equals(nil) {
			t.Errorf("expected iface1.Equals(nil) to be false")
		}

		// Test terhadap NullType (harus dianggap sama untuk assignment)
		if !iface1.Equals(NullType) {
			t.Errorf("expected iface1.Equals(NullType) to be true")
		}
	})
}
