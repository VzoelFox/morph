# Morph Memory Layout Specification

## Value Representation Strategy: Unboxed

Morph uses direct C types (no boxing/tagging) for performance.

## Type Mappings

| Morph Type    | C Type          | Size | Alignment |
|---------------|-----------------|------|-----------|
| int           | int64_t         | 8    | 8         |
| float         | double          | 8    | 8         |
| bool          | bool            | 1    | 1         |
| string        | morph_string*   | 8    | 8         |
| []T           | morph_array*    | 8    | 8         |
| map[K]V       | morph_map*      | 8    | 8         |
| Struct        | struct {...}    | Var  | Var       |

## String Representation

```c
typedef struct {
    char* data;      // UTF-8 bytes (NOT null-terminated)
    size_t length;   // Number of bytes
    size_t capacity; // For growth (unused in v1.0)
} morph_string;
```

## Array Representation

```c
typedef struct {
    void** data;       // Array of pointers to elements
    size_t length;     // Current element count
    size_t capacity;   // Allocated capacity
    size_t elem_size;  // sizeof(element)
} morph_array;
```

Growth: 2x capacity when full
Initial: 8 elements

## Map Representation

```c
typedef struct {
    morph_map_entry* buckets;
    size_t size;
    size_t capacity;
} morph_map;

typedef struct morph_map_entry {
    uint64_t hash;
    void* key;
    void* value;
    struct morph_map_entry* next;  // Collision chaining
} morph_map_entry;
```

Hash: FNV-1a
Load factor: 0.75
Resize: 2x buckets

## Struct Layout

```c
// Morph:
struktur User
    id int
    name string
    active bool
akhir

// C:
typedef struct {
    int64_t id;          // offset 0, size 8
    morph_string* name;  // offset 8, size 8
    bool active;         // offset 16, size 1
    char _pad[7];        // offset 17, align to 24
} User;  // total size 24
```

Alignment: Natural (max field alignment)
Padding: Inserted for alignment
