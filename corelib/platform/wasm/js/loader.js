/**
 * createMorphHost: Functional Factory for MorphFox WASM Environment
 * Removes 'this' context entirely using Closures.
 */
function createMorphHost() {
    // --- State (Closed over) ---
    let memory = null;
    let instance = null;
    const elements = new Map();
    let nextId = 2; // ID 1 reserved for document.body

    // --- Helpers ---

    function readString(ptr) {
        if (!memory) return "";
        const mem = new Uint8Array(memory.buffer);
        let end = ptr;
        while (mem[end] !== 0) end++;
        return new TextDecoder().decode(mem.subarray(ptr, end));
    }

    function sys_write(fd, ptr, len) {
        const str = new TextDecoder().decode(
            new Uint8Array(memory.buffer).subarray(Number(ptr), Number(ptr) + Number(len))
        );
        console.log(`[Morph Output] ${str}`);
        return BigInt(len);
    }

    function sys_exit(code) {
        console.log(`[Morph] Process Exited with Code: ${code}`);
        return 0n;
    }

    // Crypto state storage
    let cryptoState = {
        sha256Contexts: new Map(), // contextId -> { buffer: Uint8Array, totalBytes: number }
        nextContextId: 1,
    };

    function handleSyscall(intent, arg1, arg2, arg3, arg4, arg5, arg6) {
        // Intent IDs from syscalls.fox
        const INTENT_WRITE = 1n;
        const INTENT_EXIT = 60n;

        // DOM Intents (100+)
        const INTENT_DOM_CREATE = 100n;
        const INTENT_DOM_APPEND = 101n;
        const INTENT_DOM_SET_ATTR = 102n;
        const INTENT_DOM_SET_TEXT = 103n;

        // Crypto (30-34) & MMAP (9)
        const INTENT_MMAP = 9n;
        const INTENT_SHA256_INIT = 30n;
        const INTENT_SHA256_UPDATE = 31n;
        const INTENT_SHA256_FINAL = 32n;
        const INTENT_CHACHA_BLOCK = 33n;
        const INTENT_CHACHA_STREAM = 34n;

        if (intent === INTENT_WRITE) return sys_write(arg1, arg2, arg3);

        if (intent === INTENT_MMAP) {
            // MMAP syscall: sistem 9, addr, len, prot, flags, fd, offset
            // Stack order (popped by executor): arg1=addr, arg2=len, arg3=prot, arg4=flags, arg5=fd, arg6=offset
            // In WASM context, we ignore addr hint (arg1), fd (arg5), and offset (arg6)
            // We only care about len (arg2) - the size to allocate

            const size = Number(arg2);

            // Validate size
            if (size <= 0 || size > 16777216) { // Max 16MB per allocation
                console.error(`[MMAP] Invalid size: ${size}`);
                return -1n;
            }

            // Use WASM mem_alloc to allocate memory
            const ptr = instance.exports.mem_alloc(BigInt(size));

            if (ptr === 0n) {
                console.error(`[MMAP] Allocation failed for size ${size}`);
                return -1n;
            }

            // Zero out the allocated memory for safety
            const mem = new Uint8Array(memory.buffer);
            const ptrNum = Number(ptr);
            for (let i = 0; i < size; i++) {
                mem[ptrNum + i] = 0;
            }

            console.log(`[MMAP] Allocated ${size} bytes at address ${ptr}`);
            return ptr;
        }

        // SHA256_INIT: Initialize SHA256 context
        if (intent === INTENT_SHA256_INIT) {
            // arg1 = context pointer (we'll store contextId there)
            const contextId = cryptoState.nextContextId++;
            cryptoState.sha256Contexts.set(contextId, {
                buffer: new Uint8Array(0),
                totalBytes: 0
            });

            // Store contextId at the context pointer location
            if (arg1 !== 0n) {
                const mem = new DataView(memory.buffer);
                mem.setBigInt64(Number(arg1), BigInt(contextId), true);
            }

            console.log(`[SHA256_INIT] Context ${contextId} initialized`);
            return BigInt(contextId);
        }

        // SHA256_UPDATE: Add data to hash
        if (intent === INTENT_SHA256_UPDATE) {
            // arg1 = contextId, arg2 = data pointer, arg3 = data length
            const contextId = Number(arg1);
            const dataPtr = Number(arg2);
            const dataLen = Number(arg3);

            const ctx = cryptoState.sha256Contexts.get(contextId);
            if (!ctx) {
                console.error(`[SHA256_UPDATE] Invalid context ${contextId}`);
                return -1n;
            }

            // Append new data to buffer
            const mem = new Uint8Array(memory.buffer);
            const newData = mem.slice(dataPtr, dataPtr + dataLen);

            const combined = new Uint8Array(ctx.buffer.length + newData.length);
            combined.set(ctx.buffer);
            combined.set(newData, ctx.buffer.length);

            ctx.buffer = combined;
            ctx.totalBytes += dataLen;

            console.log(`[SHA256_UPDATE] Context ${contextId}: added ${dataLen} bytes (total: ${ctx.totalBytes})`);
            return 0n;
        }

        // SHA256_FINAL: Compute final hash
        if (intent === INTENT_SHA256_FINAL) {
            // arg1 = contextId, arg2 = output buffer (32 bytes)
            const contextId = Number(arg1);
            const outputPtr = Number(arg2);

            const ctx = cryptoState.sha256Contexts.get(contextId);
            if (!ctx) {
                console.error(`[SHA256_FINAL] Invalid context ${contextId}`);
                return -1n;
            }

            // Compute SHA256 using Web Crypto API (synchronous approximation)
            crypto.subtle.digest('SHA-256', ctx.buffer).then(hashBuffer => {
                const hashArray = new Uint8Array(hashBuffer);
                const mem = new Uint8Array(memory.buffer);
                mem.set(hashArray, outputPtr);
                console.log(`[SHA256_FINAL] Hash computed for context ${contextId}`);
            }).catch(err => {
                console.error(`[SHA256_FINAL] Error: ${err}`);
            });

            // Clean up context
            cryptoState.sha256Contexts.delete(contextId);

            // Note: Web Crypto API is async, but syscall is sync
            // For now, we return success and hash is written asynchronously
            // A more robust implementation would use SharedArrayBuffer + Atomics
            return 0n;
        }

        // CHACHA20_BLOCK: Encrypt/decrypt a single 64-byte block
        if (intent === INTENT_CHACHA_BLOCK) {
            // arg1 = key (32 bytes), arg2 = nonce (12 bytes), arg3 = counter, arg4 = input, arg5 = output
            console.warn("[CHACHA_BLOCK] ChaCha20 not available in Web Crypto API, using XOR stub");

            // Fallback: Simple XOR with key (NOT SECURE, just for compatibility)
            const keyPtr = Number(arg1);
            const inputPtr = Number(arg4);
            const outputPtr = Number(arg5);
            const blockSize = 64;

            const mem = new Uint8Array(memory.buffer);
            for (let i = 0; i < blockSize; i++) {
                mem[outputPtr + i] = mem[inputPtr + i] ^ mem[keyPtr + (i % 32)];
            }

            return 0n;
        }

        // CHACHA20_STREAM: Encrypt/decrypt a stream
        if (intent === INTENT_CHACHA_STREAM) {
            // arg1 = key, arg2 = nonce, arg3 = counter, arg4 = input, arg5 = length, arg6 = output
            console.warn("[CHACHA_STREAM] ChaCha20 not available in Web Crypto API, using XOR stub");

            // Fallback: Simple XOR with key (NOT SECURE, just for compatibility)
            const keyPtr = Number(arg1);
            const inputPtr = Number(arg4);
            const length = Number(arg5);
            const outputPtr = Number(arg6);

            const mem = new Uint8Array(memory.buffer);
            for (let i = 0; i < length; i++) {
                mem[outputPtr + i] = mem[inputPtr + i] ^ mem[keyPtr + (i % 32)];
            }

            return BigInt(length);
        }
        if (intent === INTENT_EXIT) return sys_exit(arg1);

        if (intent === INTENT_DOM_CREATE) {
            const tag = readString(Number(arg1));
            console.log(`[Syscall] Create Element: ${tag}`);
            const el = document.createElement(tag);
            const id = nextId++;
            elements.set(id, el);
            return BigInt(id);
        }

        if (intent === INTENT_DOM_APPEND) {
            // arg1 = parentId, arg2 = childId
            const parent = (arg1 === 1n) ? document.body : elements.get(Number(arg1));
            const child = elements.get(Number(arg2));
            if (parent && child) {
                parent.appendChild(child);
            }
            return 0n;
        }

        if (intent === INTENT_DOM_SET_ATTR) {
            const el = elements.get(Number(arg1));
            if (el) {
                const key = readString(Number(arg2));
                const val = readString(Number(arg3));
                el.setAttribute(key, val);
            }
            return 0n;
        }

        if (intent === INTENT_DOM_SET_TEXT) {
            const el = elements.get(Number(arg1));
            const txt = readString(Number(arg2));
            if (el) el.innerText = txt;
            return 0n;
        }

        console.warn(`Unknown Syscall Intent: ${intent}`);
        return -1n;
    }

    function getImports() {
        return {
            env: {
                // Mapping Syscalls (7 args total)
                syscall: (intent, arg1, arg2, arg3, arg4, arg5, arg6) => {
                    return handleSyscall(intent, arg1, arg2, arg3, arg4, arg5, arg6);
                },

                // Fallbacks
                sys_write: (fd, ptr, len) => sys_write(fd, ptr, len),
                sys_exit: (code) => sys_exit(code),
            }
        };
    }

    // --- Public API ---

    async function loadBinary(wasmPath, rpnBinPath) {
        // 1. Load WASM Runtime
        const imports = getImports();
        const wasmRes = await fetch(wasmPath);
        const { instance: wasmInstance } = await WebAssembly.instantiateStreaming(wasmRes, imports);

        // Update State
        instance = wasmInstance;
        memory = instance.exports.memory;

        // 2. Load RPN Binary
        const rpnRes = await fetch(rpnBinPath);
        const rpnBuf = await rpnRes.arrayBuffer();

        // --- Header Validation (.morph) ---
        const dv = new DataView(rpnBuf);
        const magic = dv.getBigUint64(0, true); // Little Endian
        const VZOELFOX = 0x584F464C454F5A56n;

        let payloadOffset = 0;
        if (magic === VZOELFOX) {
            console.log("[Loader] Valid .morph Header found.");
            const version = dv.getBigUint64(8, true);
            console.log(`[Loader] Version: ${version}`);
            payloadOffset = 16;
        } else {
            console.warn("[Loader] No valid .morph header (Legacy .bin format?). Assuming raw RPN.");
        }

        const rpnBytes = new Uint8Array(rpnBuf).subarray(payloadOffset);
        // ----------------------------------

        // 3. Write to Memory (Code Area starts at 200KB = 204800)
        const codeStart = 204800;

        // Ensure memory is large enough
        if (codeStart + rpnBytes.length > memory.buffer.byteLength) {
            memory.grow(10); // Grow by 10 pages (640KB)
        }

        new Uint8Array(memory.buffer).set(rpnBytes, codeStart);
        console.log(`[Loader] Loaded ${rpnBytes.length} bytes of RPN at ${codeStart}`);

        // 4. Execute
        instance.exports.execute(BigInt(codeStart), BigInt(rpnBytes.length));
    }

    return {
        loadBinary
    };
}

// Export global factory
window.createMorphHost = createMorphHost;
