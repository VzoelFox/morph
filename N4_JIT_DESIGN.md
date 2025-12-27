# FoxVM JIT Compiler Design

## JIT Strategy: Template-Based Code Generation

### Hot Path Detection
```fox
struktur HotSpot
    function_id int
    call_count int
    threshold int
akhir

fungsi (vm *FoxVM) ShouldJIT(func_id int) bool
    kembali vm.hotspots[func_id].call_count > JIT_THRESHOLD
akhir
```

### Native Code Templates
```c
// Template for ADD instruction
void jit_add_template() {
    asm volatile(
        "popq %rax\n"      // Pop second operand
        "popq %rbx\n"      // Pop first operand  
        "addq %rbx, %rax\n" // Add
        "pushq %rax\n"     // Push result
    );
}

// Template for LOAD_CONST
void jit_load_const_template(int value) {
    asm volatile(
        "movq %0, %%rax\n"   // Load constant
        "pushq %%rax\n"      // Push to stack
        : : "r"((long)value) : "rax"
    );
}
```

### JIT Compilation Process
1. **Hot path detected** → Trigger JIT compilation
2. **Bytecode analysis** → Identify instruction sequence
3. **Template selection** → Choose native code templates
4. **Code generation** → Emit x86-64 assembly
5. **Memory allocation** → Allocate executable memory
6. **Function pointer** → Replace interpreter call

### Performance Expectations
- **Interpreter**: ~1M instructions/sec
- **JIT**: ~100M instructions/sec (100x improvement)
- **Compilation overhead**: ~1ms per function
