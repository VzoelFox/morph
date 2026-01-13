// corelib/platform/wasm/js/compiler.js

class MorphCompiler {
    constructor(memory) {
        this.memory = memory;
        this.OP = { DATA: 0, ADD: 10, SYSCALL: 40 };
        this.TOKEN = { EOF: 0, INTEGER: 1, STRING: 3, IDENTIFIER: 4, SYMBOL: 5, MARKER: 8 };
    }

    readToken(ptr, index) {
        const offset = ptr + (index * 32);
        const mem64 = new BigInt64Array(this.memory.buffer);
        const idx = Math.floor(offset / 8);
        return {
            type: Number(mem64[idx]),
            value: mem64[idx + 1],
            line: mem64[idx + 2],
            col: mem64[idx + 3]
        };
    }

    readString(ptr) {
        const mem = new Uint8Array(this.memory.buffer);
        let end = Number(ptr);
        while (mem[end] !== 0) end++;
        return new TextDecoder().decode(mem.subarray(Number(ptr), end));
    }

    compile(tokenVecPtr) {
        console.log(`[Compiler] Starting...`);
        const code = [];
        let i = 0;

        // Scan tokens to demonstrate import detection logic
        while(true) {
            const token = this.readToken(Number(tokenVecPtr), i);
            if (token.type === this.TOKEN.EOF) break;

            // Log Identifiers to verify Ambil/ambil logic
            if (token.type === this.TOKEN.IDENTIFIER) {
                const idStr = this.readString(token.value);
                // console.log(`Token[${i}] ID=${idStr}`);

                if (idStr === "ambil") {
                     console.log(`[Compiler] Detected Keyword: ambil (Granular Import)`);
                     // Logic: Next token should be string "path".
                     // Emit OP_HINT or handle import.
                }

                if (idStr === "Ambil") {
                     console.log(`[Compiler] Detected Keyword: Ambil (ID Import)`);
                     // Logic: Next token should be Integer.
                     // Emit INTENT_UNIT_IMPORT.
                }
            }

            if (token.type === this.TOKEN.MARKER) {
                console.log(`[Compiler] Detected Marker: ### (Block Separator)`);
            }

            i++;
        }

        // Demo Code Gen (Hardcoded)
        // Reconstruct basic list of literals for the demo
        const tokens = [];
        i = 0;
        while(true) {
            const t = this.readToken(Number(tokenVecPtr), i++);
            if (t.type === this.TOKEN.EOF) break;
            if (t.type === this.TOKEN.STRING || t.type === this.TOKEN.INTEGER) {
                tokens.push(t);
            }
        }

        // 1. Create H1
        this.emit(code, this.OP.DATA, tokens[0].value); // "h1"
        this.emit(code, this.OP.SYSCALL, 100);

        // 2. Set Text
        this.emit(code, this.OP.DATA, tokens[1].value); // "Hello..."
        this.emit(code, this.OP.SYSCALL, 103);

        // 3. Append to Body
        this.emit(code, this.OP.DATA, 1n); // Parent (Body)
        this.emit(code, this.OP.DATA, 2n); // Child (Handle)
        this.emit(code, this.OP.SYSCALL, 101); // Append

        return this.serialize(code);
    }

    emit(code, op, arg) {
        code.push({ op: BigInt(op), arg: BigInt(arg || 0) });
    }

    serialize(code) {
        const buffer = new Uint8Array(code.length * 16);
        const view = new DataView(buffer.buffer);
        for (let i = 0; i < code.length; i++) {
            view.setBigInt64(i * 16, code[i].op, true);
            view.setBigInt64((i * 16) + 8, code[i].arg, true);
        }
        return buffer;
    }
}
window.MorphCompiler = MorphCompiler;
