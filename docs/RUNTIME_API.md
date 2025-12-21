# Morph Runtime API

## String Functions

```c
morph_string* morph_string_new(const char* cstr);
morph_string* morph_string_concat(morph_string* a, morph_string* b);
morph_string* morph_string_substr(morph_string* s, int64_t start, int64_t len);
bool morph_string_equals(morph_string* a, morph_string* b);
void morph_string_free(morph_string* s);  // Called by GC
```

## Array Functions

```c
morph_array* morph_array_new(size_t elem_size);
void morph_array_append(morph_array* arr, void* elem);
void* morph_array_get(morph_array* arr, int64_t index);
int64_t morph_array_length(morph_array* arr);
```

## Map Functions

```c
morph_map* morph_map_new();
void morph_map_set(morph_map* m, void* key, void* value);
void* morph_map_get(morph_map* m, void* key);
bool morph_map_has(morph_map* m, void* key);
```

## Memory Functions

```c
void* morph_alloc(size_t size);        // GC_malloc
void* morph_alloc_atomic(size_t size); // For byte arrays
void morph_init_gc();                  // GC_INIT
```
