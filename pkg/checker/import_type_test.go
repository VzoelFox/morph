package checker

import (
	"testing"
)

func TestImportExportedTypes(t *testing.T) {
	mock := &MockImporter{
		modules: map[string]string{
			"model": `
				struktur User
					name string
					age int
				akhir

				fungsi NewUser(n string) User
					kembalikan User{name: n, age: 0}
				akhir

				# Private struct
				struktur internalData
					val int
				akhir
			`,
		},
	}

	// 1. Use Qualified Type in Var Declaration
	input1 := `
	ambil "model"

	var u model.User
	u = model.NewUser("Alice")

	var age int = u.age
	`
	runImportTest(t, mock, input1, 0)

	// 2. Access Private Type (Should Fail)
	input2 := `
	ambil "model"
	var d model.internalData
	`
	runImportTest(t, mock, input2, 1) // Expected error: Module does not export type 'internalData'

	// 3. Invalid Qualified Type (Not a type)
	input3 := `
	ambil "model"
	var x model.NewUser # NewUser is a function, not a type
	`
	runImportTest(t, mock, input3, 1) // Expected error: 'NewUser' is not a type
}
