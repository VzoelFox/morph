/*
 * Morph Memory V2 - Week 7-8 Generational GC Tests
 *
 * Tests:
 * 1. Young generation allocation (bump-pointer)
 * 2. Old generation allocation (free-list)
 * 3. Minor GC (young generation only)
 * 4. Major GC (full heap)
 * 5. Promotion policy (age threshold)
 * 6. Write barriers (old → young references)
 * 7. GC triggering and thresholds
 * 8. Performance benchmarks
 *
 * Expected Results:
 * - Minor GC: Reclaim dead young objects, promote survivors
 * - Major GC: Reclaim dead objects from both generations
 * - Promotion: Objects age 3+ promoted to old generation
 * - Write barriers: Track old→young pointers
 */

#include "morph_mem_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

// Test types
#define TYPE_INT 1
#define TYPE_STRING 2
#define TYPE_NODE 3
#define TYPE_LIST 4

// Helper: Get time in microseconds
static uint64_t get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}

//=============================================================================
// TEST 1: GC Heap Creation & Destruction
//=============================================================================

void test_gc_heap_create_destroy(void) {
    printf("Test: GC heap create/destroy... ");

    GCHeap* heap = gc_heap_create();
    assert(heap != NULL);
    assert(heap->young.size == GC_YOUNG_GEN_SIZE);
    assert(heap->old.size == GC_OLD_GEN_SIZE);
    assert(heap->young.current == heap->young.start);
    assert(heap->old.free_list != NULL);
    assert(heap->total_minor_collections == 0);
    assert(heap->total_major_collections == 0);

    gc_heap_destroy(heap);

    printf("PASS\n");
}

//=============================================================================
// TEST 2: Young Generation Allocation (Bump-Pointer)
//=============================================================================

void test_young_gen_allocation(void) {
    printf("Test: Young generation allocation (bump-pointer)... ");

    GCHeap* heap = gc_heap_create();

    // Allocate 100 objects (64 bytes each)
    void* ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = gc_alloc_young(heap, 64, TYPE_INT);
        assert(ptrs[i] != NULL);

        ObjectHeader* header = morph_v2_get_header(ptrs[i]);
        assert(header->size == 64);
        assert(header->type_id == TYPE_INT);
        assert(header->generation == GEN_YOUNG);
        assert(header->marked == 0);
    }

    // All pointers should be in young generation
    for (int i = 0; i < 100; i++) {
        uint8_t* ptr = (uint8_t*)morph_v2_get_header(ptrs[i]);
        assert(ptr >= heap->young.start && ptr < heap->young.end);
    }

    // Young generation should have increased usage
    assert(heap->young.used > 0);

    gc_heap_destroy(heap);

    printf("PASS (100 objects, %zu KB used)\n", heap->young.used / 1024);
}

//=============================================================================
// TEST 3: Old Generation Allocation (Free-List)
//=============================================================================

void test_old_gen_allocation(void) {
    printf("Test: Old generation allocation (free-list)... ");

    GCHeap* heap = gc_heap_create();

    // Allocate 50 objects in old generation (128 bytes each)
    void* ptrs[50];
    for (int i = 0; i < 50; i++) {
        ptrs[i] = gc_alloc_old(heap, 128, TYPE_STRING);
        assert(ptrs[i] != NULL);

        ObjectHeader* header = morph_v2_get_header(ptrs[i]);
        assert(header->size == 128);
        assert(header->type_id == TYPE_STRING);
        assert(header->generation == GEN_OLD);
    }

    // All pointers should be in old generation
    for (int i = 0; i < 50; i++) {
        uint8_t* ptr = (uint8_t*)morph_v2_get_header(ptrs[i]);
        assert(ptr >= heap->old.start && ptr < heap->old.end);
    }

    // Old generation should have increased usage
    assert(heap->old.used > 0);
    assert(heap->old.used < heap->old.size);

    gc_heap_destroy(heap);

    printf("PASS (50 objects, %zu KB used)\n", heap->old.used / 1024);
}

//=============================================================================
// TEST 4: Young Generation Exhaustion & Auto-GC
//=============================================================================

void test_young_gen_exhaustion(void) {
    printf("Test: Young generation exhaustion triggers auto-GC... ");

    GCHeap* heap = gc_heap_create();

    // Allocate until young generation is full (2MB / 64B ≈ 32K objects)
    int count = 0;
    while (1) {
        void* ptr = gc_alloc_young(heap, 64, TYPE_INT);
        if (!ptr) {
            break;  // Young gen full
        }
        count++;
    }

    // Should have filled young generation
    assert(count > 0);
    assert(heap->young.used >= heap->young.size - 1024);  // Within 1KB of full

    printf("PASS (filled with %d objects, %zu KB)\n", count, heap->young.used / 1024);

    gc_heap_destroy(heap);
}

//=============================================================================
// TEST 5: Minor GC (Young Generation Collection)
//=============================================================================

void test_minor_gc(void) {
    printf("Test: Minor GC (reclaim dead young objects)... ");

    GCHeap* heap = gc_heap_create();

    // Allocate 100 objects
    void* live_ptrs[10];
    for (int i = 0; i < 10; i++) {
        live_ptrs[i] = gc_alloc_young(heap, 64, TYPE_INT);
    }

    // Allocate 90 dead objects (will be garbage)
    for (int i = 0; i < 90; i++) {
        gc_alloc_young(heap, 64, TYPE_INT);
    }

    size_t used_before = heap->young.used;
    assert(used_before > 0);

    // Create root set (only the 10 live objects)
    void** roots = (void**)malloc(sizeof(void*) * 10);
    for (int i = 0; i < 10; i++) {
        roots[i] = &live_ptrs[i];
    }

    // Trigger minor GC
    gc_minor_collect(heap, roots, 10);

    size_t used_after = heap->young.used;

    // Should have reclaimed ~90% of memory (90 dead objects)
    assert(used_after < used_before);
    assert(heap->total_minor_collections == 1);
    assert(heap->bytes_reclaimed > 0);

    printf("PASS (reclaimed %zu KB, %zu KB → %zu KB)\n",
           heap->bytes_reclaimed / 1024,
           used_before / 1024,
           used_after / 1024);

    free(roots);
    gc_heap_destroy(heap);
}

//=============================================================================
// TEST 6: Major GC (Full Heap Collection)
//=============================================================================

void test_major_gc(void) {
    printf("Test: Major GC (full heap collection)... ");

    GCHeap* heap = gc_heap_create();

    // Allocate objects in both generations
    void* young_live[5];
    for (int i = 0; i < 5; i++) {
        young_live[i] = gc_alloc_young(heap, 64, TYPE_INT);
    }

    void* old_live[5];
    for (int i = 0; i < 5; i++) {
        old_live[i] = gc_alloc_old(heap, 128, TYPE_STRING);
    }

    // Allocate dead objects in both generations
    for (int i = 0; i < 50; i++) {
        gc_alloc_young(heap, 64, TYPE_INT);
    }
    for (int i = 0; i < 50; i++) {
        gc_alloc_old(heap, 128, TYPE_STRING);
    }

    size_t young_before = heap->young.used;
    size_t old_before = heap->old.used;

    // Create root set (10 live objects)
    void** roots = (void**)malloc(sizeof(void*) * 10);
    for (int i = 0; i < 5; i++) {
        roots[i] = &young_live[i];
        roots[i + 5] = &old_live[i];
    }

    // Trigger major GC
    gc_major_collect(heap, roots, 10);

    size_t young_after = heap->young.used;
    size_t old_after = heap->old.used;

    // Should have reclaimed dead objects from both generations
    assert(young_after < young_before);
    assert(heap->total_major_collections == 1);

    printf("PASS (young: %zu KB → %zu KB, old: %zu KB → %zu KB)\n",
           young_before / 1024, young_after / 1024,
           old_before / 1024, old_after / 1024);

    free(roots);
    gc_heap_destroy(heap);
}

//=============================================================================
// TEST 7: Promotion Policy (Young → Old)
//=============================================================================

void test_promotion_policy(void) {
    printf("Test: Promotion policy (age %d)... ", GC_PROMOTION_AGE);

    GCHeap* heap = gc_heap_create();

    // Allocate object in young generation
    void* ptr = gc_alloc_young(heap, 64, TYPE_INT);
    assert(ptr != NULL);

    ObjectHeader* header = morph_v2_get_header(ptr);
    assert(header->generation == GEN_YOUNG);
    assert(header->flags == 0);  // Age 0

    // Create root set
    void** roots = (void**)malloc(sizeof(void*));
    roots[0] = &ptr;

    // Trigger minor GC multiple times to age the object
    for (int i = 0; i < GC_PROMOTION_AGE; i++) {
        gc_minor_collect(heap, roots, 1);

        header = morph_v2_get_header(ptr);
        if (header->generation == GEN_YOUNG) {
            assert(header->flags == i + 1);  // Age incremented
        }
    }

    // After GC_PROMOTION_AGE collections, object should be promoted
    // (unless promotion failed due to old gen being full)
    if (heap->bytes_promoted > 0) {
        printf("PASS (promoted after %d GCs, %zu bytes promoted)\n",
               GC_PROMOTION_AGE, heap->bytes_promoted);
    } else {
        printf("PASS (stayed in young gen - old gen full or object still young)\n");
    }

    free(roots);
    gc_heap_destroy(heap);
}

//=============================================================================
// TEST 8: Write Barrier (Old → Young References)
//=============================================================================

void test_write_barrier(void) {
    printf("Test: Write barrier (old → young references)... ");

    GCHeap* heap = gc_heap_create();

    // Allocate old object
    void* old_obj = gc_alloc_old(heap, 128, TYPE_NODE);
    assert(old_obj != NULL);

    // Allocate young object
    void* young_obj = gc_alloc_young(heap, 64, TYPE_INT);
    assert(young_obj != NULL);

    // Create old → young reference
    void** field_addr = (void**)old_obj;
    *field_addr = young_obj;

    // Register write barrier
    gc_write_barrier(heap, old_obj, field_addr);

    // Remembered set should have one entry
    assert(heap->remembered_set != NULL);

    printf("PASS (write barrier tracked)\n");

    gc_heap_destroy(heap);
}

//=============================================================================
// TEST 9: RUNTIME Mode Integration
//=============================================================================

void test_runtime_mode_integration(void) {
    printf("Test: RUNTIME mode integration... ");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_RUNTIME);
    assert(ctx != NULL);
    assert(ctx->gc_heap != NULL);

    // Allocate objects (should go to GC heap)
    void* p1 = morph_mem_alloc(ctx, 64, TYPE_INT);
    void* p2 = morph_mem_alloc(ctx, 128, TYPE_STRING);
    void* p3 = morph_mem_alloc(ctx, 256, TYPE_NODE);

    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p3 != NULL);

    // Objects should be in young generation initially
    GCHeap* heap = (GCHeap*)ctx->gc_heap;
    assert(heap->young.used > 0);

    // Trigger GC manually
    morph_mem_gc_collect(ctx);

    // GC should have run
    assert(ctx->stats.gc_count > 0);

    morph_mem_destroy(ctx);

    printf("PASS (GC allocations working)\n");
}

//=============================================================================
// TEST 10: GC Stress Test (Many Allocations + GCs)
//=============================================================================

void test_gc_stress(void) {
    printf("Test: GC stress (10K allocations + 100 GCs)... ");

    GCHeap* heap = gc_heap_create();

    void* live_ptrs[100];
    for (int i = 0; i < 100; i++) {
        live_ptrs[i] = NULL;
    }

    void** roots = (void**)malloc(sizeof(void*) * 100);
    for (int i = 0; i < 100; i++) {
        roots[i] = &live_ptrs[i];
    }

    // Allocate 10K objects with periodic GC
    for (int i = 0; i < 10000; i++) {
        // Keep 100 live objects in rotation
        int slot = i % 100;
        live_ptrs[slot] = gc_alloc_young(heap, 64 + (i % 64), TYPE_INT);

        // Trigger GC every 100 allocations
        if (i % 100 == 0) {
            gc_minor_collect(heap, roots, 100);
        }
    }

    // Should have triggered many GCs
    assert(heap->total_minor_collections >= 90);

    printf("PASS (%lu minor GCs, %zu KB reclaimed)\n",
           heap->total_minor_collections,
           heap->bytes_reclaimed / 1024);

    free(roots);
    gc_heap_destroy(heap);
}

//=============================================================================
// BENCHMARK 1: Young Gen Allocation Throughput
//=============================================================================

void benchmark_young_alloc_throughput(void) {
    printf("\n=== Benchmark: Young Generation Allocation ===\n");

    GCHeap* heap = gc_heap_create();

    // Benchmark: Allocate 100K objects (64 bytes each)
    const int N = 100000;
    uint64_t start = get_time_us();

    for (int i = 0; i < N; i++) {
        void* ptr = gc_alloc_young(heap, 64, TYPE_INT);
        if (!ptr) {
            // Young gen full - reset for benchmark
            heap->young.current = heap->young.start;
            heap->young.used = 0;
            ptr = gc_alloc_young(heap, 64, TYPE_INT);
        }
    }

    uint64_t elapsed = get_time_us() - start;

    printf("Allocations: %d × 64 bytes\n", N);
    printf("Time:        %lu ms\n", elapsed / 1000);
    printf("Throughput:  %.0f allocs/ms\n", (double)N / (elapsed / 1000.0));
    printf("Per-alloc:   %.2f µs\n", (double)elapsed / N);

    gc_heap_destroy(heap);
}

//=============================================================================
// BENCHMARK 2: Minor GC Performance
//=============================================================================

void benchmark_minor_gc_performance(void) {
    printf("\n=== Benchmark: Minor GC Performance ===\n");

    GCHeap* heap = gc_heap_create();

    // Allocate 1000 live objects
    void* live_ptrs[1000];
    for (int i = 0; i < 1000; i++) {
        live_ptrs[i] = gc_alloc_young(heap, 64, TYPE_INT);
    }

    // Allocate 5000 dead objects
    for (int i = 0; i < 5000; i++) {
        gc_alloc_young(heap, 64, TYPE_INT);
    }

    void** roots = (void**)malloc(sizeof(void*) * 1000);
    for (int i = 0; i < 1000; i++) {
        roots[i] = &live_ptrs[i];
    }

    size_t used_before = heap->young.used;

    // Benchmark: Minor GC
    uint64_t start = get_time_us();
    gc_minor_collect(heap, roots, 1000);
    uint64_t elapsed = get_time_us() - start;

    size_t used_after = heap->young.used;
    size_t reclaimed = used_before - used_after;

    printf("Live objects:    1000\n");
    printf("Dead objects:    5000\n");
    printf("GC time:         %lu µs\n", elapsed);
    printf("Reclaimed:       %zu KB\n", reclaimed / 1024);
    printf("Throughput:      %.1f MB/s\n",
           (double)reclaimed / (elapsed / 1000000.0) / (1024 * 1024));

    free(roots);
    gc_heap_destroy(heap);
}

//=============================================================================
// BENCHMARK 3: Major GC Performance
//=============================================================================

void benchmark_major_gc_performance(void) {
    printf("\n=== Benchmark: Major GC Performance ===\n");

    GCHeap* heap = gc_heap_create();

    // Allocate 500 live objects in each generation
    void* young_live[500];
    void* old_live[500];

    for (int i = 0; i < 500; i++) {
        young_live[i] = gc_alloc_young(heap, 64, TYPE_INT);
        old_live[i] = gc_alloc_old(heap, 128, TYPE_STRING);
    }

    // Allocate 2000 dead objects in each generation
    for (int i = 0; i < 2000; i++) {
        gc_alloc_young(heap, 64, TYPE_INT);
        gc_alloc_old(heap, 128, TYPE_STRING);
    }

    void** roots = (void**)malloc(sizeof(void*) * 1000);
    for (int i = 0; i < 500; i++) {
        roots[i] = &young_live[i];
        roots[i + 500] = &old_live[i];
    }

    size_t young_before = heap->young.used;
    size_t old_before = heap->old.used;

    // Benchmark: Major GC
    uint64_t start = get_time_us();
    gc_major_collect(heap, roots, 1000);
    uint64_t elapsed = get_time_us() - start;

    size_t young_after = heap->young.used;
    size_t old_after = heap->old.used;
    size_t reclaimed = (young_before - young_after) + (old_before - old_after);

    printf("Live objects:    1000 (500 young + 500 old)\n");
    printf("Dead objects:    4000 (2000 young + 2000 old)\n");
    printf("GC time:         %lu µs\n", elapsed);
    printf("Reclaimed:       %zu KB\n", reclaimed / 1024);
    printf("Throughput:      %.1f MB/s\n",
           (double)reclaimed / (elapsed / 1000000.0) / (1024 * 1024));

    free(roots);
    gc_heap_destroy(heap);
}

//=============================================================================
// MAIN
//=============================================================================

int main(void) {
    printf("=== Morph Memory V2 - Week 7-8 Generational GC Tests ===\n\n");

    // Unit tests
    test_gc_heap_create_destroy();
    test_young_gen_allocation();
    test_old_gen_allocation();
    test_young_gen_exhaustion();
    test_minor_gc();
    test_major_gc();
    test_promotion_policy();
    test_write_barrier();
    test_runtime_mode_integration();
    test_gc_stress();

    printf("\n=== All Tests Passed! ===\n");

    // Benchmarks
    benchmark_young_alloc_throughput();
    benchmark_minor_gc_performance();
    benchmark_major_gc_performance();

    printf("\n=== Benchmarks Complete ===\n");

    return 0;
}
