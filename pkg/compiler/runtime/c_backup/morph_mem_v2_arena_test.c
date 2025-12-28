/*
 * Morph Memory V2 - Week 2 Arena Tests
 * Test arena allocator functionality and performance
 */

#include "morph_mem_v2.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

// Test type IDs
#define TYPE_INT 1
#define TYPE_STRING 2
#define TYPE_ARRAY 3
#define TYPE_AST_NODE 4

// ANSI color codes
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

//=============================================================================
// ARENA UNIT TESTS
//=============================================================================

void test_arena_create_destroy(void) {
    printf("Test: Arena create/destroy...");

    Arena* arena = arena_create(ARENA_BLOCK_SIZE);
    assert(arena != NULL);
    assert(arena->current != NULL);
    assert(arena->first != NULL);
    assert(arena->block_size == ARENA_BLOCK_SIZE);
    assert(arena->total_allocated == ARENA_BLOCK_SIZE);
    assert(arena->total_used == 0);

    arena_destroy(arena);

    printf(GREEN " PASS\n" RESET);
}

void test_arena_basic_alloc(void) {
    printf("Test: Arena basic allocation...");

    Arena* arena = arena_create(ARENA_BLOCK_SIZE);

    // Allocate small objects
    void* p1 = arena_alloc(arena, 32);
    assert(p1 != NULL);
    assert(arena->total_used >= 32);

    void* p2 = arena_alloc(arena, 64);
    assert(p2 != NULL);
    assert(arena->total_used >= 96);

    void* p3 = arena_alloc(arena, 128);
    assert(p3 != NULL);

    // Verify no overlap
    assert((char*)p2 >= (char*)p1 + 32);
    assert((char*)p3 >= (char*)p2 + 64);

    arena_destroy(arena);

    printf(GREEN " PASS\n" RESET);
}

void test_arena_alignment(void) {
    printf("Test: Arena alignment...");

    Arena* arena = arena_create(ARENA_BLOCK_SIZE);

    // All allocations should be 8-byte aligned
    for (int i = 0; i < 100; i++) {
        void* ptr = arena_alloc(arena, 1 + i);  // Varying sizes
        assert(ptr != NULL);
        assert(((size_t)ptr % 8) == 0);  // 8-byte aligned
    }

    arena_destroy(arena);

    printf(GREEN " PASS\n" RESET);
}

void test_arena_large_alloc(void) {
    printf("Test: Arena large allocation (multi-block)...");

    Arena* arena = arena_create(ARENA_BLOCK_SIZE);

    // Allocate 1MB (should fit in first block)
    void* p1 = arena_alloc(arena, 1024 * 1024);
    assert(p1 != NULL);
    assert(arena->current == arena->first);

    // Allocate another 1.5MB (should trigger new block)
    void* p2 = arena_alloc(arena, 1024 * 1024 + 512 * 1024);
    assert(p2 != NULL);
    assert(arena->current != arena->first);  // New block created
    assert(arena->first->next != NULL);

    arena_destroy(arena);

    printf(GREEN " PASS\n" RESET);
}

void test_arena_reset(void) {
    printf("Test: Arena reset...");

    Arena* arena = arena_create(ARENA_BLOCK_SIZE);

    // Allocate many objects
    for (int i = 0; i < 1000; i++) {
        arena_alloc(arena, 100);
    }

    size_t used_before = arena->total_used;
    assert(used_before > 0);

    // Reset
    arena_reset(arena);

    // Verify reset
    assert(arena->total_used == 0);
    assert(arena->current == arena->first);
    assert(arena->first->used == 0);

    // Should be able to allocate again
    void* p1 = arena_alloc(arena, 64);
    assert(p1 != NULL);

    arena_destroy(arena);

    printf(GREEN " PASS\n" RESET);
}

void test_arena_custom_block_size(void) {
    printf("Test: Arena custom block size...");

    size_t custom_size = 512 * 1024;  // 512KB
    Arena* arena = arena_create(custom_size);

    assert(arena->block_size == custom_size);
    assert(arena->total_allocated == custom_size);

    arena_destroy(arena);

    printf(GREEN " PASS\n" RESET);
}

//=============================================================================
// INTEGRATION TESTS (with MorphContextV2)
//=============================================================================

void test_compiler_mode_uses_arena(void) {
    printf("Test: COMPILER mode uses arena...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    assert(ctx != NULL);
    assert(ctx->allocator_data != NULL);  // Arena created

    // Allocate objects
    void* p1 = morph_mem_alloc(ctx, 100, TYPE_INT);
    void* p2 = morph_mem_alloc(ctx, 200, TYPE_STRING);
    void* p3 = morph_mem_alloc(ctx, 300, TYPE_ARRAY);

    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p3 != NULL);

    // Verify stats
    MorphMemStats stats = morph_mem_get_stats(ctx);
    assert(stats.object_count == 3);
    assert(stats.arena_bytes >= 600);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_compiler_mode_no_gc(void) {
    printf("Test: COMPILER mode - no GC overhead...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);

    // Allocate many objects (shouldn't trigger GC)
    for (int i = 0; i < 10000; i++) {
        morph_mem_alloc(ctx, 32, TYPE_INT);
    }

    MorphMemStats stats = morph_mem_get_stats(ctx);
    assert(stats.gc_count == 0);  // No GC in COMPILER mode

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_mixed_allocations(void) {
    printf("Test: Mixed allocation sizes...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    morph_mem_register_type(ctx, TYPE_INT, "Int");
    morph_mem_register_type(ctx, TYPE_STRING, "String");
    morph_mem_register_type(ctx, TYPE_ARRAY, "Array");

    // Mix of small, medium, large
    for (int i = 0; i < 100; i++) {
        morph_mem_alloc(ctx, 16, TYPE_INT);        // Small
        morph_mem_alloc(ctx, 256, TYPE_STRING);    // Medium
        morph_mem_alloc(ctx, 4096, TYPE_ARRAY);    // Large
    }

    MorphMemStats stats = morph_mem_get_stats(ctx);
    assert(stats.object_count == 300);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

//=============================================================================
// PERFORMANCE BENCHMARKS
//=============================================================================

void benchmark_arena_vs_malloc(void) {
    printf("\n" CYAN "=== Arena vs Malloc Benchmark ===" RESET "\n");

    const int NUM_ALLOCS = 100000;
    const size_t ALLOC_SIZE = 64;

    // Benchmark malloc
    clock_t start = clock();
    void** ptrs = (void**)malloc(sizeof(void*) * NUM_ALLOCS);
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(ALLOC_SIZE);
    }
    clock_t malloc_time = clock() - start;

    // Free malloc allocations
    for (int i = 0; i < NUM_ALLOCS; i++) {
        free(ptrs[i]);
    }
    free(ptrs);

    // Benchmark arena
    start = clock();
    Arena* arena = arena_create(ARENA_BLOCK_SIZE);
    for (int i = 0; i < NUM_ALLOCS; i++) {
        arena_alloc(arena, ALLOC_SIZE);
    }
    clock_t arena_time = clock() - start;
    arena_destroy(arena);

    // Results
    double malloc_ms = (double)malloc_time * 1000.0 / CLOCKS_PER_SEC;
    double arena_ms = (double)arena_time * 1000.0 / CLOCKS_PER_SEC;
    double speedup = malloc_ms / arena_ms;

    printf("  Allocations: %d x %zu bytes\n", NUM_ALLOCS, ALLOC_SIZE);
    printf("  Malloc:      %.2f ms\n", malloc_ms);
    printf("  Arena:       %.2f ms\n", arena_ms);
    printf("  " GREEN "Speedup:     %.2fx faster" RESET "\n", speedup);

    assert(arena_ms < malloc_ms);  // Arena should be faster
}

void benchmark_arena_utilization(void) {
    printf("\n" CYAN "=== Arena Memory Utilization ===" RESET "\n");

    Arena* arena = arena_create(ARENA_BLOCK_SIZE);

    // Simulate compilation workload (varying sizes)
    size_t total_payload = 0;
    for (int i = 0; i < 10000; i++) {
        size_t size = 16 + (i % 256);  // 16-272 bytes
        arena_alloc(arena, size);
        total_payload += size;
    }

    double utilization = 100.0 * arena->total_used / arena->total_allocated;

    printf("  Total allocated: %zu KB\n", arena->total_allocated / 1024);
    printf("  Total used:      %zu KB\n", arena->total_used / 1024);
    printf("  Payload:         %zu KB\n", total_payload / 1024);
    printf("  " GREEN "Utilization:     %.1f%%" RESET "\n", utilization);

    // Good utilization is >70%
    assert(utilization > 70.0);

    arena_destroy(arena);
}

void benchmark_compiler_mode(void) {
    printf("\n" CYAN "=== COMPILER Mode Benchmark ===" RESET "\n");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    morph_mem_register_type(ctx, TYPE_AST_NODE, "ASTNode");

    clock_t start = clock();

    // Simulate AST node allocation during parsing
    for (int i = 0; i < 50000; i++) {
        // Mix of small (tokens) and larger (AST nodes)
        morph_mem_alloc(ctx, 32, TYPE_INT);       // Token
        morph_mem_alloc(ctx, 128, TYPE_AST_NODE); // AST node
    }

    clock_t elapsed = clock() - start;
    double ms = (double)elapsed * 1000.0 / CLOCKS_PER_SEC;

    MorphMemStats stats = morph_mem_get_stats(ctx);

    printf("  Objects:     %lu\n", stats.object_count);
    printf("  Memory:      %lu KB\n", stats.current_live / 1024);
    printf("  Time:        %.2f ms\n", ms);
    printf("  " GREEN "Throughput:  %.0f allocs/ms" RESET "\n",
           stats.object_count / ms);

    morph_mem_destroy(ctx);
}

//=============================================================================
// MAIN
//=============================================================================

int main(void) {
    printf("\n=== Morph Memory V2 - Week 2 Arena Tests ===\n\n");

    // Unit tests
    printf("--- Arena Unit Tests ---\n");
    test_arena_create_destroy();
    test_arena_basic_alloc();
    test_arena_alignment();
    test_arena_large_alloc();
    test_arena_reset();
    test_arena_custom_block_size();

    // Integration tests
    printf("\n--- Integration Tests ---\n");
    test_compiler_mode_uses_arena();
    test_compiler_mode_no_gc();
    test_mixed_allocations();

    printf("\n" GREEN "=== All Tests Passed! ===" RESET "\n");

    // Performance benchmarks
    benchmark_arena_vs_malloc();
    benchmark_arena_utilization();
    benchmark_compiler_mode();

    printf("\n" GREEN "=== Week 2 Complete! ===" RESET "\n\n");

    return 0;
}
