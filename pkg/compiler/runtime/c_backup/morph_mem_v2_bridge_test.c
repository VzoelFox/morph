/*
 * Morph Memory V2 - Bridge Integration Test
 * Week 5-6: Tests V1-to-V2 bridging layer
 */

#include "morph_mem_v2_bridge.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// ANSI colors
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

// Test types (mimicking V1 types)
MorphTypeInfo test_ti_int = { "TestInt", sizeof(int64_t), 0, NULL, NULL };
MorphTypeInfo test_ti_string = { "TestString", sizeof(MorphString), 0, NULL, NULL };
MorphTypeInfo test_ti_array = { "TestArray", sizeof(MorphArray), 0, NULL, NULL };

//=============================================================================
// BRIDGE INTEGRATION TESTS
//=============================================================================

void test_bridge_init_destroy(void) {
    printf("Test: Bridge init/destroy...");

    MorphContextBridge* bridge = mph_bridge_init();
    assert(bridge != NULL);
    assert(bridge->using_v2 == 1);  // Should use V2
    assert(bridge->v2_ctx != NULL);

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

void test_bridge_type_registration(void) {
    printf("Test: Bridge type registration...");

    MorphContextBridge* bridge = mph_bridge_init();

    // Register types
    uint8_t id1 = mph_bridge_register_type(bridge, &test_ti_int);
    uint8_t id2 = mph_bridge_register_type(bridge, &test_ti_string);
    uint8_t id3 = mph_bridge_register_type(bridge, &test_ti_array);

    assert(id1 > 0);
    assert(id2 > 0);
    assert(id3 > 0);
    assert(id1 != id2);
    assert(id2 != id3);

    // Verify reverse mapping
    assert(mph_bridge_get_type(bridge, id1) == &test_ti_int);
    assert(mph_bridge_get_type(bridge, id2) == &test_ti_string);
    assert(mph_bridge_get_type(bridge, id3) == &test_ti_array);

    // Re-register same type should return same ID
    uint8_t id1_again = mph_bridge_register_type(bridge, &test_ti_int);
    assert(id1_again == id1);

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

void test_bridge_allocation(void) {
    printf("Test: Bridge allocation...");

    MorphContextBridge* bridge = mph_bridge_init();

    // Allocate using V1-style API
    int64_t* p1 = (int64_t*)mph_bridge_alloc(bridge, &test_ti_int);
    MorphString* p2 = (MorphString*)mph_bridge_alloc(bridge, &test_ti_string);
    MorphArray* p3 = (MorphArray*)mph_bridge_alloc(bridge, &test_ti_array);

    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p3 != NULL);

    // Verify zero-initialization (V1 compatibility)
    assert(*p1 == 0);
    assert(p2->data == NULL);
    assert(p2->length == 0);
    assert(p3->data == NULL);
    assert(p3->length == 0);

    // Verify allocation counts
    assert(bridge->allocation_count == 3);
    assert(bridge->v2_alloc_count == 3);

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

void test_bridge_many_allocations(void) {
    printf("Test: Bridge many allocations...");

    MorphContextBridge* bridge = mph_bridge_init();

    // Allocate many small objects (should use pool)
    for (int i = 0; i < 1000; i++) {
        int64_t* ptr = (int64_t*)mph_bridge_alloc(bridge, &test_ti_int);
        assert(ptr != NULL);
        *ptr = i;
    }

    // Allocate larger objects (should use arena)
    MorphTypeInfo large_type = { "LargeObject", 1024, 0, NULL, NULL };
    for (int i = 0; i < 100; i++) {
        void* ptr = mph_bridge_alloc(bridge, &large_type);
        assert(ptr != NULL);
    }

    assert(bridge->allocation_count == 1100);

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

void test_bridge_gc_roots(void) {
    printf("Test: Bridge GC root management...");

    MorphContextBridge* bridge = mph_bridge_init();

    // Allocate objects
    void* obj1 = mph_bridge_alloc(bridge, &test_ti_int);
    void* obj2 = mph_bridge_alloc(bridge, &test_ti_string);
    void* obj3 = mph_bridge_alloc(bridge, &test_ti_array);

    // Push as GC roots
    mph_bridge_gc_push_root(bridge, &obj1);
    mph_bridge_gc_push_root(bridge, &obj2);
    mph_bridge_gc_push_root(bridge, &obj3);

    // Pop roots
    mph_bridge_gc_pop_roots(bridge, 3);

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

void test_bridge_gc_collect(void) {
    printf("Test: Bridge GC collect...");

    MorphContextBridge* bridge = mph_bridge_init();

    // Allocate objects
    for (int i = 0; i < 100; i++) {
        mph_bridge_alloc(bridge, &test_ti_int);
    }

    // Trigger GC (should be no-op in COMPILER mode)
    mph_bridge_gc_collect(bridge);

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

void test_bridge_stats(void) {
    printf("Test: Bridge statistics...");

    MorphContextBridge* bridge = mph_bridge_init();

    // Allocate various objects
    for (int i = 0; i < 50; i++) {
        mph_bridge_alloc(bridge, &test_ti_int);
        mph_bridge_alloc(bridge, &test_ti_string);
    }

    // Print stats (should not crash)
    mph_bridge_print_stats(bridge);

    // Dump stats to file
    mph_bridge_dump_stats(bridge, "/tmp/morph_bridge_stats.json");

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

//=============================================================================
// V1 COMPATIBILITY TESTS
//=============================================================================

void test_v1_compatibility_macros(void) {
    printf("Test: V1 compatibility macros...");

    MorphContextBridge* bridge = mph_bridge_init();

    // These macros should work like V1 API
    void* obj1 = mph_alloc(bridge, &test_ti_int);
    void* obj2 = mph_alloc(bridge, &test_ti_string);

    assert(obj1 != NULL);
    assert(obj2 != NULL);

    mph_gc_push_root(bridge, &obj1);
    mph_gc_push_root(bridge, &obj2);
    mph_gc_pop_roots(bridge, 2);

    mph_gc_collect(bridge);

    mph_bridge_destroy(bridge);

    printf(GREEN " PASS\n" RESET);
}

//=============================================================================
// PERFORMANCE COMPARISON
//=============================================================================

void benchmark_bridge_allocation(void) {
    printf("\n" CYAN "=== Bridge Allocation Benchmark ===" RESET "\n");

    MorphContextBridge* bridge = mph_bridge_init();

    clock_t start = clock();

    // Simulate compilation workload
    for (int i = 0; i < 10000; i++) {
        // Many small objects (tokens)
        mph_bridge_alloc(bridge, &test_ti_int);  // 8 bytes
        mph_bridge_alloc(bridge, &test_ti_int);

        // Fewer larger objects (AST nodes)
        MorphTypeInfo node_type = { "ASTNode", 256, 0, NULL, NULL };
        mph_bridge_alloc(bridge, &node_type);
    }

    clock_t elapsed = clock() - start;
    double ms = (double)elapsed * 1000.0 / CLOCKS_PER_SEC;

    printf("  Allocations: 30,000 (20K small + 10K medium)\n");
    printf("  Time:        %.2f ms\n", ms);
    printf("  " GREEN "Throughput:  %.0f allocs/ms" RESET "\n", 30000.0 / ms);

    mph_bridge_destroy(bridge);
}

void benchmark_bridge_overhead(void) {
    printf("\n" CYAN "=== Bridge Overhead Benchmark ===" RESET "\n");

    MorphContextBridge* bridge = mph_bridge_init();

    // Measure pure allocation overhead (type registration cached)
    uint8_t type_id = mph_bridge_register_type(bridge, &test_ti_int);

    clock_t start = clock();

    for (int i = 0; i < 100000; i++) {
        mph_bridge_alloc(bridge, &test_ti_int);
    }

    clock_t elapsed = clock() - start;
    double ms = (double)elapsed * 1000.0 / CLOCKS_PER_SEC;

    printf("  Allocations: 100,000 x 8 bytes\n");
    printf("  Time:        %.2f ms\n", ms);
    printf("  " GREEN "Overhead:    %.3f µs/alloc" RESET "\n", (ms * 1000.0) / 100000.0);

    mph_bridge_destroy(bridge);
}

//=============================================================================
// MAIN
//=============================================================================

int main(void) {
    printf("\n=== Morph Memory V2 - Bridge Integration Tests ===\n\n");

    // Integration tests
    printf("--- Bridge Tests ---\n");
    test_bridge_init_destroy();
    test_bridge_type_registration();
    test_bridge_allocation();
    test_bridge_many_allocations();
    test_bridge_gc_roots();
    test_bridge_gc_collect();
    test_bridge_stats();

    // Compatibility tests
    printf("\n--- V1 Compatibility Tests ---\n");
    test_v1_compatibility_macros();

    printf("\n" GREEN "=== All Tests Passed! ===" RESET "\n");

    // Performance benchmarks
    benchmark_bridge_allocation();
    benchmark_bridge_overhead();

    printf("\n" GREEN "=== Bridge Integration Complete! ===" RESET "\n\n");

    return 0;
}
