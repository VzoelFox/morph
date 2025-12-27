# FoxVM Bytecode Specification

## Instruction Set Architecture

### Stack-Based VM
- **Stack**: Primary data structure
- **Locals**: Local variable storage
- **Constants**: Constant pool

### Core Instructions
```
LOAD_CONST <index>     # Push constant to stack
LOAD_LOCAL <index>     # Push local variable to stack
STORE_LOCAL <index>    # Pop stack to local variable
ADD                    # Pop two values, push sum
SUB                    # Pop two values, push difference
MUL                    # Pop two values, push product
DIV                    # Pop two values, push quotient
CALL <func_index>      # Call function
RETURN                 # Return from function
JUMP <offset>          # Unconditional jump
JUMP_IF_FALSE <offset> # Conditional jump
PRINT                  # Pop and print value
HALT                   # Stop execution
```

### Bytecode Format
```
[Instruction] [Operand1] [Operand2] ...
```

### Example Compilation
```fox
# Morph source
var x = 10
var y = 20
var result = x + y
native_print_int(result)
```

```
# FoxVM bytecode
LOAD_CONST 0    # 10
STORE_LOCAL 0   # x
LOAD_CONST 1    # 20
STORE_LOCAL 1   # y
LOAD_LOCAL 0    # x
LOAD_LOCAL 1    # y
ADD
STORE_LOCAL 2   # result
LOAD_LOCAL 2    # result
PRINT
HALT
```
