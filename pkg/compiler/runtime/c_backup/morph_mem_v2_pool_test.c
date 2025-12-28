/*
 * Morph Memory V2 - Week 3-4 Pool Tests
 * Test pool allocator functionality and reuse
 */

#include "morph_mem_v2.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

// Test type IDs
#define TYPE_TOKEN 1
#define TYPE_MOVE 2
#define TYPE_NODE 3
#define TYPE_EXPR 4

// ANSI color codes
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

// Move struct (matches number_chess_stress.fox)
typedef struct {
    int from_pos;
    int to_pos;
    int captured;
} ChessMove;

//=============================================================================
// POOL UNIT TESTS
//=============================================================================

void test_pool_manager_create_destroy(void) {
    printf("Test: PoolManager create/destroy...");

    PoolManager* mgr = pool_manager_create();
    assert(mgr != NULL);

    // Verify all pools initialized
    for (int i = 0; i < POOL_NUM_SIZES; i++) {
        assert(mgr->pools[i].object_size > 0);
        assert(mgr->pools[i].objects_per_slab > 0);
        assert(mgr->pools[i].free_list == NULL);  // No slabs yet
        assert(mgr->pools[i].total_allocated == 0);
    }

    pool_manager_destroy(mgr);

    printf(GREEN " PASS\n" RESET);
}

void test_pool_size_class_mapping(void) {
    printf("Test: Pool size class mapping...");

    assert(pool_get_size_class(1) == 0);    // 1 byte → 16-byte pool
    assert(pool_get_size_class(16) == 0);   // 16 bytes → 16-byte pool
    assert(pool_get_size_class(17) == 1);   // 17 bytes → 32-byte pool
    assert(pool_get_size_class(32) == 1);   // 32 bytes → 32-byte pool
    assert(pool_get_size_class(64) == 2);   // 64 bytes → 64-byte pool
    assert(pool_get_size_class(128) == 3);  // 128 bytes → 128-byte pool
    assert(pool_get_size_class(256) == 4);  // 256 bytes → 256-byte pool
    assert(pool_get_size_class(257) == -1); // Too large

    printf(GREEN " PASS\n" RESET);
}

void test_pool_basic_alloc(void) {
    printf("Test: Pool basic allocation...");

    PoolManager* mgr = pool_manager_create();

    // Allocate from each size class
    void* p16 = pool_alloc(mgr, 16);
    void* p32 = pool_alloc(mgr, 32);
    void* p64 = pool_alloc(mgr, 64);
    void* p128 = pool_alloc(mgr, 128);
    void* p256 = pool_alloc(mgr, 256);

    assert(p16 != NULL);
    assert(p32 != NULL);
    assert(p64 != NULL);
    assert(p128 != NULL);
    assert(p256 != NULL);

    // Verify slabs allocated
    assert(mgr->pools[0].slabs != NULL);
    assert(mgr->pools[1].slabs != NULL);
    assert(mgr->pools[2].slabs != NULL);
    assert(mgr->pools[3].slabs != NULL);
    assert(mgr->pools[4].slabs != NULL);

    pool_manager_destroy(mgr);

    printf(GREEN " PASS\n" RESET);
}

void test_pool_alloc_free_reuse(void) {
    printf("Test: Pool alloc/free/reuse...");

    PoolManager* mgr = pool_manager_create();

    // Allocate object
    void* p1 = pool_alloc(mgr, 64);
    assert(p1 != NULL);
    size_t used_before = mgr->pools[2].total_used;
    assert(used_before == 1);

    // Free it
    pool_free(mgr, p1, 64);
    assert(mgr->pools[2].total_used == 0);
    assert(mgr->pools[2].total_free == mgr->pools[2].objects_per_slab);

    // Allocate again - should reuse same slot!
    void* p2 = pool_alloc(mgr, 64);
    assert(p2 == p1);  // Same pointer reused!
    assert(mgr->pools[2].total_used == 1);

    pool_manager_destroy(mgr);

    printf(GREEN " PASS\n" RESET);
}

void test_pool_multiple_slabs(void) {
    printf("Test: Pool multiple slab allocation...");

    PoolManager* mgr = pool_manager_create();

    // Allocate more than one slab worth (64-byte pool has 1000 objects/slab)
    int num_allocs = POOL_OBJECTS_PER_SLAB_64 + 500;
    void** ptrs = (void**)malloc(sizeof(void*) * num_allocs);

    for (int i = 0; i < num_allocs; i++) {
        ptrs[i] = pool_alloc(mgr, 64);
        assert(ptrs[i] != NULL);
    }

    // Should have allocated 2 slabs
    Pool* pool = &mgr->pools[2];
    int slab_count = 0;
    PoolSlab* slab = pool->slabs;
    while (slab) {
        slab_count++;
        slab = slab->next;
    }
    assert(slab_count == 2);

    free(ptrs);
    pool_manager_destroy(mgr);

    printf(GREEN " PASS\n" RESET);
}

void test_pool_free_list_integrity(void) {
    printf("Test: Pool free list integrity...");

    PoolManager* mgr = pool_manager_create();

    // Allocate and free in alternating pattern
    void* p1 = pool_alloc(mgr, 32);
    void* p2 = pool_alloc(mgr, 32);
    void* p3 = pool_alloc(mgr, 32);

    pool_free(mgr, p2, 32);  // Free middle one
    void* p4 = pool_alloc(mgr, 32);
    assert(p4 == p2);  // Should reuse p2's slot

    pool_free(mgr, p1, 32);
    pool_free(mgr, p3, 32);
    void* p5 = pool_alloc(mgr, 32);
    void* p6 = pool_alloc(mgr, 32);
    assert((p5 == p1 && p6 == p3) || (p5 == p3 && p6 == p1));  // Order may vary

    pool_manager_destroy(mgr);

    printf(GREEN " PASS\n" RESET);
}

//=============================================================================
// INTEGRATION TESTS (with MorphContextV2)
//=============================================================================

void test_compiler_mode_uses_pool(void) {
    printf("Test: COMPILER mode uses pool for small objects...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    assert(ctx != NULL);

    // Allocate small objects (should use pool)
    void* p1 = morph_mem_alloc(ctx, 16, TYPE_TOKEN);
    void* p2 = morph_mem_alloc(ctx, 32, TYPE_TOKEN);
    void* p3 = morph_mem_alloc(ctx, 64, TYPE_MOVE);

    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p3 != NULL);

    // Verify pool stats
    MorphMemStats stats = morph_mem_get_stats(ctx);
    assert(stats.pool_bytes > 0);
    assert(stats.object_count == 3);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_hybrid_pool_arena_allocation(void) {
    printf("Test: Hybrid pool+arena allocation...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);

    // Small objects → pool
    void* small1 = morph_mem_alloc(ctx, 16, TYPE_TOKEN);
    void* small2 = morph_mem_alloc(ctx, 64, TYPE_TOKEN);
    void* small3 = morph_mem_alloc(ctx, 128, TYPE_MOVE);

    // Large objects → arena
    void* large1 = morph_mem_alloc(ctx, 512, TYPE_NODE);
    void* large2 = morph_mem_alloc(ctx, 1024, TYPE_EXPR);

    MorphMemStats stats = morph_mem_get_stats(ctx);
    assert(stats.pool_bytes > 0);   // Pool used for small
    assert(stats.arena_bytes > 0);  // Arena used for large

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_pool_with_free(void) {
    printf("Test: Pool allocation with explicit free...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);

    // Allocate
    void* p1 = morph_mem_alloc(ctx, 32, TYPE_TOKEN);
    void* p2 = morph_mem_alloc(ctx, 32, TYPE_TOKEN);

    MorphMemStats stats1 = morph_mem_get_stats(ctx);
    size_t count_before = stats1.object_count;

    // Free
    morph_mem_free(ctx, p1);

    MorphMemStats stats2 = morph_mem_get_stats(ctx);
    assert(stats2.total_freed == 32);

    // Allocate again - should reuse
    void* p3 = morph_mem_alloc(ctx, 32, TYPE_TOKEN);
    assert(p3 == p1);  // Reused!

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

//=============================================================================
// CHESS STRESS TEST (simulates number_chess_stress.fox)
//=============================================================================

void test_chess_move_allocation(void) {
    printf("Test: Chess move allocation pattern...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    morph_mem_register_type(ctx, TYPE_MOVE, "ChessMove");

    // ChessMove is 12 bytes (3 ints), ObjectHeader is 16 bytes
    // Total: 28 bytes → rounds to 32 bytes → uses 32-byte pool
    ChessMove moves[30];
    void* move_ptrs[30];

    // Allocate 30 moves (like number_chess_stress.fox)
    for (int i = 0; i < 30; i++) {
        move_ptrs[i] = morph_mem_alloc(ctx, sizeof(ChessMove), TYPE_MOVE);
        assert(move_ptrs[i] != NULL);

        ChessMove* m = (ChessMove*)move_ptrs[i];
        m->from_pos = i;
        m->to_pos = i + 1;
        m->captured = 0;
    }

    // Simulate undo (free in reverse)
    for (int i = 29; i >= 0; i--) {
        ChessMove* m = (ChessMove*)move_ptrs[i];
        assert(m->from_pos == i);  // Verify data integrity
        morph_mem_free(ctx, move_ptrs[i]);
    }

    // Allocate again - should reuse slots
    for (int i = 0; i < 30; i++) {
        void* ptr = morph_mem_alloc(ctx, sizeof(ChessMove), TYPE_MOVE);
        assert(ptr != NULL);
        // Verify it's one of the original pointers (reused)
        int found = 0;
        for (int j = 0; j < 30; j++) {
            if (ptr == move_ptrs[j]) {
                found = 1;
                break;
            }
        }
        assert(found);  // Pointer was reused!
    }

    MorphMemStats stats = morph_mem_get_stats(ctx);
    assert(stats.object_count_by_type[TYPE_MOVE] == 30);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

//=============================================================================
// PERFORMANCE BENCHMARKS
//=============================================================================

void benchmark_pool_vs_malloc(void) {
    printf("\n" CYAN "=== Pool vs Malloc Benchmark ===" RESET "\n");

    const int NUM_ALLOCS = 50000;
    const size_t ALLOC_SIZE = 64;

    // Benchmark malloc
    clock_t start = clock();
    void** ptrs = (void**)malloc(sizeof(void*) * NUM_ALLOCS);
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(ALLOC_SIZE);
    }
    for (int i = 0; i < NUM_ALLOCS; i++) {
        free(ptrs[i]);
    }
    clock_t malloc_time = clock() - start;
    free(ptrs);

    // Benchmark pool
    start = clock();
    PoolManager* mgr = pool_manager_create();
    ptrs = (void**)malloc(sizeof(void*) * NUM_ALLOCS);
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = pool_alloc(mgr, ALLOC_SIZE);
    }
    for (int i = 0; i < NUM_ALLOCS; i++) {
        pool_free(mgr, ptrs[i], ALLOC_SIZE);
    }
    clock_t pool_time = clock() - start;
    free(ptrs);
    pool_manager_destroy(mgr);

    // Results
    double malloc_ms = (double)malloc_time * 1000.0 / CLOCKS_PER_SEC;
    double pool_ms = (double)pool_time * 1000.0 / CLOCKS_PER_SEC;
    double speedup = malloc_ms / pool_ms;

    printf("  Allocations: %d x %zu bytes (with free)\n", NUM_ALLOCS, ALLOC_SIZE);
    printf("  Malloc:      %.2f ms\n", malloc_ms);
    printf("  Pool:        %.2f ms\n", pool_ms);
    printf("  " GREEN "Speedup:     %.2fx faster" RESET "\n", speedup);
}

void benchmark_pool_reuse_pattern(void) {
    printf("\n" CYAN "=== Pool Reuse Pattern Benchmark ===" RESET "\n");

    PoolManager* mgr = pool_manager_create();

    clock_t start = clock();

    // Allocate/free/reallocate pattern (simulates chess moves)
    for (int iter = 0; iter < 1000; iter++) {
        void* ptrs[100];

        // Allocate 100 objects
        for (int i = 0; i < 100; i++) {
            ptrs[i] = pool_alloc(mgr, 64);
        }

        // Free all
        for (int i = 0; i < 100; i++) {
            pool_free(mgr, ptrs[i], 64);
        }
    }

    clock_t elapsed = clock() - start;
    double ms = (double)elapsed * 1000.0 / CLOCKS_PER_SEC;

    printf("  Iterations:  1000 x (alloc 100 + free 100)\n");
    printf("  Time:        %.2f ms\n", ms);
    printf("  " GREEN "Throughput:  %.0f ops/ms" RESET "\n", (1000.0 * 200.0) / ms);

    pool_manager_destroy(mgr);
}

void benchmark_hybrid_allocation(void) {
    printf("\n" CYAN "=== Hybrid Pool+Arena Benchmark ===" RESET "\n");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    morph_mem_register_type(ctx, TYPE_TOKEN, "Token");
    morph_mem_register_type(ctx, TYPE_NODE, "ASTNode");

    clock_t start = clock();

    // Realistic compilation pattern
    for (int i = 0; i < 10000; i++) {
        // Many small tokens (16-32 bytes) → pool
        morph_mem_alloc(ctx, 16, TYPE_TOKEN);
        morph_mem_alloc(ctx, 24, TYPE_TOKEN);
        morph_mem_alloc(ctx, 32, TYPE_TOKEN);

        // Fewer larger AST nodes (512 bytes) → arena
        morph_mem_alloc(ctx, 512, TYPE_NODE);
    }

    clock_t elapsed = clock() - start;
    double ms = (double)elapsed * 1000.0 / CLOCKS_PER_SEC;

    MorphMemStats stats = morph_mem_get_stats(ctx);

    printf("  Objects:      %lu (30K tokens + 10K nodes)\n", stats.object_count);
    printf("  Pool memory:  %lu KB\n", stats.pool_bytes / 1024);
    printf("  Arena memory: %lu KB\n", stats.arena_bytes / 1024);
    printf("  Time:         %.2f ms\n", ms);
    printf("  " GREEN "Throughput:   %.0f allocs/ms" RESET "\n", stats.object_count / ms);

    morph_mem_destroy(ctx);
}

//=============================================================================
// MAIN
//=============================================================================

int main(void) {
    printf("\n=== Morph Memory V2 - Week 3-4 Pool Tests ===\n\n");

    // Unit tests
    printf("--- Pool Unit Tests ---\n");
    test_pool_manager_create_destroy();
    test_pool_size_class_mapping();
    test_pool_basic_alloc();
    test_pool_alloc_free_reuse();
    test_pool_multiple_slabs();
    test_pool_free_list_integrity();

    // Integration tests
    printf("\n--- Integration Tests ---\n");
    test_compiler_mode_uses_pool();
    test_hybrid_pool_arena_allocation();
    test_pool_with_free();

    // Chess stress test
    printf("\n--- Chess Stress Test ---\n");
    test_chess_move_allocation();

    printf("\n" GREEN "=== All Tests Passed! ===" RESET "\n");

    // Performance benchmarks
    benchmark_pool_vs_malloc();
    benchmark_pool_reuse_pattern();
    benchmark_hybrid_allocation();

    printf("\n" GREEN "=== Week 3-4 Complete! ===" RESET "\n\n");

    return 0;
}
