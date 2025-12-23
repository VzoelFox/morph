# Tiered Memory & Garbage Collection Specification

## 1. Overview
Morph implements a **Tiered Memory Management** system to support long-running processes (self-hosting).
- **L1 (RAM)**: Managed by a Mark-and-Sweep Garbage Collector.
- **L2 (Disk/Swap)**: Managed by a "Cold Data Eviction" Daemon, storing objects in `.z` binary files.

## 2. Object Header Layout
Every heap-allocated object (Struct, Array, String, Closure) is prefixed with a header.

```c
typedef struct ObjectHeader {
    struct ObjectHeader* next;  // Global linked list for Sweep
    struct MorphTypeInfo* type; // RTTI for GC/Serialization
    uint8_t flags;              // Mark bit, Pinned bit, Swapped bit
    uint32_t last_access;       // Timestamp for LRU Eviction
} ObjectHeader;

// Flags:
// 0x01: MARKED (Reachable)
// 0x02: SWAPPED (Data is on disk)
// 0x04: PINNED (Cannot be swapped/moved)
```

## 3. Runtime Type Information (RTTI)
To traverse the graph (GC) or serialize to disk, the runtime must know which fields are pointers.

```c
typedef struct MorphTypeInfo {
    const char* name;
    size_t size;           // Size of the data payload
    int num_pointers;      // Number of pointer fields
    size_t* pointer_offsets; // Array of offsets to pointer fields
} MorphTypeInfo;
```

## 4. Garbage Collection (Mark-and-Sweep)
### Roots
- Stack variables (need Compiler to generate "Stack Maps" or use Shadow Stack).
- Global variables.
- Registry (Modules).

### Algorithm
1.  **Stop-The-World**: Pause execution.
2.  **Mark**:
    - Push roots to stack.
    - While stack not empty:
        - Pop object. Set `MARKED` bit.
        - Use `type->pointer_offsets` to find children.
        - Push unmarked children.
3.  **Sweep**:
    - Iterate `heap_head` linked list.
    - If `MARKED`: Unset bit.
    - If `!MARKED`: Free memory, remove from list.

## 5. .z Binary Format (Phase 2)
Used for swapping cold objects to `.morph.vz/swap/`.

| Offset | Type     | Description       |
|--------|----------|-------------------|
| 0x00   | 4 bytes  | Magic "MRPH"      |
| 0x04   | 8 bytes  | Type ID Hash      |
| 0x0C   | 4 bytes  | Payload Size (N)  |
| 0x10   | N bytes  | Raw Data          |

## 6. Daemon GC
- Runs on `mph_alloc` trigger (e.g., every 1MB allocated).
- Checks `last_access`. If `(now - last_access) > THRESHOLD`:
    - Serialize to `.z`.
    - Free RAM payload.
    - Set `SWAPPED` flag.
    - Keep Header in RAM (as a "Tombstone").

Accessing a `SWAPPED` object triggers a **Page Fault** handler in runtime:
1.  Load `.z` file.
2.  Alloc RAM.
3.  Deserialize.
4.  Unset `SWAPPED`.
