/*
 * Morph Memory System V2 - GC Optimization Tests
 * Week 12: Testing for Precise Tracing, Mark-Compact, Heap Resizing
 *
 * Tests:
 * - Precise tracing with type descriptors
 * - Mark-compact GC (compaction + pointer fixup)
 * - Dynamic heap resizing (grow/shrink)
 * - Integration tests
 * - Performance benchmarks
 */

#include "morph_mem_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

//=============================================================================
// TEST HELPERS
//=============================================================================

// Color codes for output
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) \
    printf(COLOR_BLUE "[TEST] %s" COLOR_RESET "\n", name)

#define TEST_PASS(name) \
    do { \
        tests_passed++; \
        printf(COLOR_GREEN "[PASS] %s" COLOR_RESET "\n\n", name); \
    } while (0)

#define TEST_FAIL(name, msg) \
    do { \
        tests_failed++; \
        printf(COLOR_RED "[FAIL] %s: %s" COLOR_RESET "\n\n", name, msg); \
    } while (0)

// Get current time in microseconds
static uint64_t get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

// Test type IDs
#define TYPE_INT 1
#define TYPE_NODE 2
#define TYPE_LIST 3
#define TYPE_TREE 4

// Test structures for precise tracing
typedef struct TestNode {
    int value;
    struct TestNode* next;
} TestNode;

typedef struct TestTree {
    int value;
    struct TestTree* left;
    struct TestTree* right;
} TestTree;

typedef struct TestList {
    int count;
    void* items[8];  // Array of pointers
} TestList;

//=============================================================================
// PRECISE TRACING TESTS
//=============================================================================

void test_type_descriptor_registration(void) {
    TEST_START("Type Descriptor Registration");

    GCHeap* heap = gc_heap_create();
    assert(heap != NULL);

    // Create type descriptor for TestNode
    TypeDescriptor node_desc = {
        .type_id = TYPE_NODE,
        .name = "TestNode",
        .size = sizeof(TestNode),
        .num_pointers = 1,
        .pointer_offsets = { offsetof(TestNode, next) }
    };

    // Register type
    gc_register_type_descriptor(heap, &node_desc);

    // Verify registration
    const TypeDescriptor* retrieved = gc_get_type_descriptor(heap, TYPE_NODE);
    assert(retrieved != NULL);
    assert(retrieved->type_id == TYPE_NODE);
    assert(strcmp(retrieved->name, "TestNode") == 0);
    assert(retrieved->size == sizeof(TestNode));
    assert(retrieved->num_pointers == 1);
    assert(retrieved->pointer_offsets[0] == offsetof(TestNode, next));

    gc_heap_destroy(heap);
    TEST_PASS("Type Descriptor Registration");
}

void test_precise_tracing_linked_list(void) {
    TEST_START("Precise Tracing - Linked List");

    GCHeap* heap = gc_heap_create();

    // Register TestNode type
    TypeDescriptor node_desc = {
        .type_id = TYPE_NODE,
        .name = "TestNode",
        .size = sizeof(TestNode),
        .num_pointers = 1,
        .pointer_offsets = { offsetof(TestNode, next) }
    };
    gc_register_type_descriptor(heap, &node_desc);

    // Create linked list: node1 -> node2 -> node3
    TestNode* node1 = (TestNode*)gc_alloc_young(heap, sizeof(TestNode), TYPE_NODE);
    TestNode* node2 = (TestNode*)gc_alloc_young(heap, sizeof(TestNode), TYPE_NODE);
    TestNode* node3 = (TestNode*)gc_alloc_young(heap, sizeof(TestNode), TYPE_NODE);

    node1->value = 1;
    node1->next = node2;
    node2->value = 2;
    node2->next = node3;
    node3->value = 3;
    node3->next = NULL;

    // Create root set (only node1 is root)
    void* roots[1] = { &node1 };

    // Allocate garbage to trigger GC
    for (int i = 0; i < 100; i++) {
        gc_alloc_young(heap, 64, TYPE_INT);
    }

    // Run minor GC with precise tracing
    size_t used_before = heap->young.used;
    gc_minor_collect(heap, roots, 1);
    size_t used_after = heap->young.used;

    // Verify all nodes still reachable
    assert(node1->value == 1);
    assert(node1->next == node2);
    assert(node2->value == 2);
    assert(node2->next == node3);
    assert(node3->value == 3);

    // Should have reclaimed garbage
    assert(used_after < used_before);

    gc_heap_destroy(heap);
    TEST_PASS("Precise Tracing - Linked List");
}

void test_precise_tracing_tree(void) {
    TEST_START("Precise Tracing - Binary Tree");

    GCHeap* heap = gc_heap_create();

    // Register TestTree type (2 pointer fields)
    TypeDescriptor tree_desc = {
        .type_id = TYPE_TREE,
        .name = "TestTree",
        .size = sizeof(TestTree),
        .num_pointers = 2,
        .pointer_offsets = {
            offsetof(TestTree, left),
            offsetof(TestTree, right)
        }
    };
    gc_register_type_descriptor(heap, &tree_desc);

    // Create binary tree
    //       root(1)
    //      /      \
    //   left(2)  right(3)
    TestTree* root = (TestTree*)gc_alloc_young(heap, sizeof(TestTree), TYPE_TREE);
    TestTree* left = (TestTree*)gc_alloc_young(heap, sizeof(TestTree), TYPE_TREE);
    TestTree* right = (TestTree*)gc_alloc_young(heap, sizeof(TestTree), TYPE_TREE);

    root->value = 1;
    root->left = left;
    root->right = right;
    left->value = 2;
    left->left = NULL;
    left->right = NULL;
    right->value = 3;
    right->left = NULL;
    right->right = NULL;

    // Root set
    void* roots[1] = { &root };

    // Allocate garbage
    for (int i = 0; i < 100; i++) {
        gc_alloc_young(heap, 128, TYPE_INT);
    }

    // Run GC
    gc_minor_collect(heap, roots, 1);

    // Verify tree intact
    assert(root->value == 1);
    assert(root->left == left);
    assert(root->right == right);
    assert(left->value == 2);
    assert(right->value == 3);

    gc_heap_destroy(heap);
    TEST_PASS("Precise Tracing - Binary Tree");
}

void test_precise_tracing_multiple_pointers(void) {
    TEST_START("Precise Tracing - Array of Pointers");

    GCHeap* heap = gc_heap_create();

    // Register TestList type (8 pointer fields)
    TypeDescriptor list_desc = {
        .type_id = TYPE_LIST,
        .name = "TestList",
        .size = sizeof(TestList),
        .num_pointers = 8,
        .pointer_offsets = {
            offsetof(TestList, items[0]),
            offsetof(TestList, items[1]),
            offsetof(TestList, items[2]),
            offsetof(TestList, items[3]),
            offsetof(TestList, items[4]),
            offsetof(TestList, items[5]),
            offsetof(TestList, items[6]),
            offsetof(TestList, items[7])
        }
    };
    gc_register_type_descriptor(heap, &list_desc);

    // Create list with 5 items
    TestList* list = (TestList*)gc_alloc_young(heap, sizeof(TestList), TYPE_LIST);
    list->count = 5;
    for (int i = 0; i < 5; i++) {
        list->items[i] = gc_alloc_young(heap, 32, TYPE_INT);
    }
    for (int i = 5; i < 8; i++) {
        list->items[i] = NULL;
    }

    // Root set
    void* roots[1] = { &list };

    // Allocate garbage
    for (int i = 0; i < 200; i++) {
        gc_alloc_young(heap, 64, TYPE_INT);
    }

    // Run GC
    gc_minor_collect(heap, roots, 1);

    // Verify all items reachable
    assert(list->count == 5);
    for (int i = 0; i < 5; i++) {
        assert(list->items[i] != NULL);
    }

    gc_heap_destroy(heap);
    TEST_PASS("Precise Tracing - Array of Pointers");
}

//=============================================================================
// MARK-COMPACT TESTS
//=============================================================================

void test_mark_compact_basic(void) {
    TEST_START("Mark-Compact - Basic Compaction");

    GCHeap* heap = gc_heap_create();

    // Allocate objects in old generation
    void* obj1 = gc_alloc_old(heap, 64, TYPE_INT);
    void* obj2 = gc_alloc_old(heap, 128, TYPE_INT);
    void* obj3 = gc_alloc_old(heap, 64, TYPE_INT);
    void* obj4 = gc_alloc_old(heap, 256, TYPE_INT);

    // Mark obj1, obj3, obj4 as live (obj2 is garbage)
    ObjectHeader* h1 = morph_v2_get_header(obj1);
    ObjectHeader* h3 = morph_v2_get_header(obj3);
    ObjectHeader* h4 = morph_v2_get_header(obj4);
    h1->marked = 1;
    h3->marked = 1;
    h4->marked = 1;

    size_t used_before = heap->old.used;

    // Compact old generation
    gc_compact_old_generation(heap);

    size_t used_after = heap->old.used;

    // Should reclaim obj2's space
    assert(used_after < used_before);
    assert(heap->bytes_compacted > 0);

    gc_heap_destroy(heap);
    TEST_PASS("Mark-Compact - Basic Compaction");
}

void test_mark_compact_pointer_fixup(void) {
    TEST_START("Mark-Compact - Pointer Fixup");

    GCHeap* heap = gc_heap_create();

    // Register TestNode type for precise pointer tracking
    TypeDescriptor node_desc = {
        .type_id = TYPE_NODE,
        .name = "TestNode",
        .size = sizeof(TestNode),
        .num_pointers = 1,
        .pointer_offsets = { offsetof(TestNode, next) }
    };
    gc_register_type_descriptor(heap, &node_desc);

    // Allocate linked list in old generation
    TestNode* node1 = (TestNode*)gc_alloc_old(heap, sizeof(TestNode), TYPE_NODE);
    TestNode* node2 = (TestNode*)gc_alloc_old(heap, sizeof(TestNode), TYPE_NODE);
    TestNode* node3 = (TestNode*)gc_alloc_old(heap, sizeof(TestNode), TYPE_NODE);

    node1->value = 1;
    node1->next = node2;
    node2->value = 2;
    node2->next = node3;
    node3->value = 3;
    node3->next = NULL;

    // Save original addresses
    void* orig_node1 = node1;
    void* orig_node2 = node2;
    void* orig_node3 = node3;

    // Mark all as live
    morph_v2_get_header(node1)->marked = 1;
    morph_v2_get_header(node2)->marked = 1;
    morph_v2_get_header(node3)->marked = 1;

    // Root set
    void* roots[1] = { &node1 };

    // Compact (will move objects)
    gc_compact_old_generation(heap);
    gc_update_references(heap, roots, 1);

    // Pointers should be updated (may have moved)
    // But values should be intact
    assert(node1->value == 1);
    assert(node1->next != NULL);
    assert(node1->next->value == 2);
    assert(node1->next->next != NULL);
    assert(node1->next->next->value == 3);

    // At least some objects should have moved
    // (compaction should eliminate fragmentation)
    printf("  Original node1: %p, After compact: %p\n", orig_node1, node1);
    printf("  Original node2: %p, After compact: %p\n", orig_node2, node1->next);
    printf("  Original node3: %p, After compact: %p\n", orig_node3, node1->next->next);

    gc_heap_destroy(heap);
    TEST_PASS("Mark-Compact - Pointer Fixup");
}

void test_mark_compact_fragmentation(void) {
    TEST_START("Mark-Compact - Fragmentation Elimination");

    GCHeap* heap = gc_heap_create();

    // Create fragmented heap: [Live][Dead][Live][Dead][Live]
    void* live1 = gc_alloc_old(heap, 128, TYPE_INT);
    void* dead1 = gc_alloc_old(heap, 128, TYPE_INT);
    void* live2 = gc_alloc_old(heap, 128, TYPE_INT);
    void* dead2 = gc_alloc_old(heap, 128, TYPE_INT);
    void* live3 = gc_alloc_old(heap, 128, TYPE_INT);

    // Mark only live objects
    morph_v2_get_header(live1)->marked = 1;
    morph_v2_get_header(live2)->marked = 1;
    morph_v2_get_header(live3)->marked = 1;

    (void)dead1;
    (void)dead2;

    size_t used_before = heap->old.used;

    // Compact
    gc_compact_old_generation(heap);

    size_t used_after = heap->old.used;

    // Should reclaim dead space
    assert(used_after < used_before);

    // Free list should be consolidated
    assert(heap->old.free_list != NULL);

    // Should be able to allocate large contiguous block
    void* large = gc_alloc_old(heap, 1024, TYPE_INT);
    assert(large != NULL);

    gc_heap_destroy(heap);
    TEST_PASS("Mark-Compact - Fragmentation Elimination");
}

//=============================================================================
// HEAP RESIZING TESTS
//=============================================================================

void test_heap_resize_young_grow(void) {
    TEST_START("Heap Resize - Young Generation Grow");

    GCHeap* heap = gc_heap_create();

    size_t original_size = heap->young.size;
    size_t new_size = original_size * 2;

    // Allocate some objects
    void* obj = gc_alloc_young(heap, 64, TYPE_INT);
    assert(obj != NULL);

    // Resize young generation
    gc_resize_young_generation(heap, new_size);

    // Verify resize
    assert(heap->young.size == new_size);
    assert(heap->young.end - heap->young.start == (ptrdiff_t)new_size);

    // Should still be able to allocate
    void* obj2 = gc_alloc_young(heap, 128, TYPE_INT);
    assert(obj2 != NULL);

    gc_heap_destroy(heap);
    TEST_PASS("Heap Resize - Young Generation Grow");
}

void test_heap_resize_young_shrink(void) {
    TEST_START("Heap Resize - Young Generation Shrink");

    GCHeap* heap = gc_heap_create();

    size_t original_size = heap->young.size;
    size_t new_size = original_size / 2;

    // Resize young generation (shrink)
    gc_resize_young_generation(heap, new_size);

    // Verify resize
    assert(heap->young.size == new_size);

    // Should still be able to allocate
    void* obj = gc_alloc_young(heap, 64, TYPE_INT);
    assert(obj != NULL);

    gc_heap_destroy(heap);
    TEST_PASS("Heap Resize - Young Generation Shrink");
}

void test_heap_resize_old_grow(void) {
    TEST_START("Heap Resize - Old Generation Grow");

    GCHeap* heap = gc_heap_create();

    size_t original_size = heap->old.size;
    size_t new_size = original_size * 2;

    // Allocate object in old gen
    void* obj = gc_alloc_old(heap, 256, TYPE_INT);
    assert(obj != NULL);

    // Resize old generation
    gc_resize_old_generation(heap, new_size);

    // Verify resize
    assert(heap->old.size == new_size);

    // Free list should be updated
    assert(heap->old.free_list != NULL);

    // Should be able to allocate large object
    void* large = gc_alloc_old(heap, 10 * 1024, TYPE_INT);
    assert(large != NULL);

    gc_heap_destroy(heap);
    TEST_PASS("Heap Resize - Old Generation Grow");
}

void test_heap_auto_resize(void) {
    TEST_START("Heap Auto-Resize");

    GCHeap* heap = gc_heap_create();

    // Configure auto-resize
    HeapResizeConfig config = {
        .min_young_size = 1 * 1024 * 1024,   // 1MB
        .max_young_size = 8 * 1024 * 1024,   // 8MB
        .min_old_size = 16 * 1024 * 1024,    // 16MB
        .max_old_size = 128 * 1024 * 1024,   // 128MB
        .grow_threshold = 0.8,                // Grow at 80%
        .shrink_threshold = 0.3               // Shrink at 30%
    };

    heap->resize_config = &config;

    // Fill young gen to 90% (trigger grow)
    size_t target_usage = (size_t)(heap->young.size * 0.9);
    while (heap->young.used < target_usage) {
        gc_alloc_young(heap, 64, TYPE_INT);
    }

    size_t size_before = heap->young.size;

    // Auto-resize should grow
    gc_auto_resize_heap(heap, &config);

    // Should have grown
    assert(heap->young.size > size_before);
    printf("  Young gen: %zu MB -> %zu MB\n",
           size_before / (1024*1024),
           heap->young.size / (1024*1024));

    heap->resize_config = NULL;
    gc_heap_destroy(heap);
    TEST_PASS("Heap Auto-Resize");
}

//=============================================================================
// INTEGRATION TESTS
//=============================================================================

void test_integration_precise_tracing_with_compaction(void) {
    TEST_START("Integration - Precise Tracing + Compaction");

    GCHeap* heap = gc_heap_create();

    // Register TestNode type
    TypeDescriptor node_desc = {
        .type_id = TYPE_NODE,
        .name = "TestNode",
        .size = sizeof(TestNode),
        .num_pointers = 1,
        .pointer_offsets = { offsetof(TestNode, next) }
    };
    gc_register_type_descriptor(heap, &node_desc);

    // Create linked list in old generation
    TestNode* head = (TestNode*)gc_alloc_old(heap, sizeof(TestNode), TYPE_NODE);
    TestNode* current = head;

    // Create 10-node list with gaps (garbage nodes)
    for (int i = 0; i < 10; i++) {
        current->value = i;

        // Allocate next node
        TestNode* next = (TestNode*)gc_alloc_old(heap, sizeof(TestNode), TYPE_NODE);
        current->next = next;
        current = next;

        // Allocate garbage between nodes
        gc_alloc_old(heap, 128, TYPE_INT);
    }
    current->next = NULL;

    // Mark list as live
    current = head;
    while (current) {
        morph_v2_get_header(current)->marked = 1;
        current = current->next;
    }

    // Root set
    void* roots[1] = { &head };

    // Compact (should eliminate garbage, preserve list structure)
    gc_compact_old_generation(heap);
    gc_update_references(heap, roots, 1);

    // Verify list integrity after compaction
    current = head;
    for (int i = 0; i < 10; i++) {
        assert(current != NULL);
        assert(current->value == i);
        current = current->next;
    }
    assert(current == NULL);

    gc_heap_destroy(heap);
    TEST_PASS("Integration - Precise Tracing + Compaction");
}

void test_integration_full_gc_with_resizing(void) {
    TEST_START("Integration - Full GC + Auto-Resize");

    GCHeap* heap = gc_heap_create();

    // Configure auto-resize
    HeapResizeConfig config = {
        .min_young_size = 1 * 1024 * 1024,
        .max_young_size = 8 * 1024 * 1024,
        .min_old_size = 16 * 1024 * 1024,
        .max_old_size = 128 * 1024 * 1024,
        .grow_threshold = 0.8,
        .shrink_threshold = 0.3
    };
    heap->resize_config = &config;

    // Allocate many objects
    void* live_objects[100];
    for (int i = 0; i < 100; i++) {
        live_objects[i] = gc_alloc_young(heap, 128, TYPE_INT);
    }

    // Allocate garbage
    for (int i = 0; i < 1000; i++) {
        gc_alloc_young(heap, 64, TYPE_INT);
    }

    // Root set
    void** roots = (void**)malloc(sizeof(void*) * 100);
    for (int i = 0; i < 100; i++) {
        roots[i] = &live_objects[i];
    }

    size_t used_before = heap->young.used;

    // Run minor GC
    gc_minor_collect(heap, roots, 100);

    // Auto-resize based on usage
    gc_auto_resize_heap(heap, &config);

    size_t used_after = heap->young.used;

    // Should reclaim garbage
    assert(used_after < used_before);

    // Live objects should still be accessible
    for (int i = 0; i < 100; i++) {
        assert(live_objects[i] != NULL);
    }

    free(roots);
    heap->resize_config = NULL;
    gc_heap_destroy(heap);
    TEST_PASS("Integration - Full GC + Auto-Resize");
}

//=============================================================================
// BENCHMARKS
//=============================================================================

void benchmark_precise_tracing_overhead(void) {
    printf(COLOR_YELLOW "[BENCHMARK] Precise Tracing Overhead" COLOR_RESET "\n");

    GCHeap* heap = gc_heap_create();

    // Register TestNode type
    TypeDescriptor node_desc = {
        .type_id = TYPE_NODE,
        .name = "TestNode",
        .size = sizeof(TestNode),
        .num_pointers = 1,
        .pointer_offsets = { offsetof(TestNode, next) }
    };
    gc_register_type_descriptor(heap, &node_desc);

    // Create large linked list (1000 nodes)
    TestNode* head = (TestNode*)gc_alloc_young(heap, sizeof(TestNode), TYPE_NODE);
    TestNode* current = head;
    for (int i = 0; i < 999; i++) {
        TestNode* next = (TestNode*)gc_alloc_young(heap, sizeof(TestNode), TYPE_NODE);
        current->next = next;
        current = next;
    }
    current->next = NULL;

    void* roots[1] = { &head };

    // Benchmark GC with precise tracing
    uint64_t start = get_time_us();
    gc_minor_collect(heap, roots, 1);
    uint64_t elapsed = get_time_us() - start;

    printf("  1000-node list GC: %lu us\n", elapsed);
    printf("  Per-node overhead: %.2f us\n", (double)elapsed / 1000.0);

    gc_heap_destroy(heap);
    printf("\n");
}

void benchmark_mark_compact_performance(void) {
    printf(COLOR_YELLOW "[BENCHMARK] Mark-Compact Performance" COLOR_RESET "\n");

    GCHeap* heap = gc_heap_create();

    // Fill old generation with live/dead objects
    void* live_objects[500];
    for (int i = 0; i < 500; i++) {
        live_objects[i] = gc_alloc_old(heap, 256, TYPE_INT);
        gc_alloc_old(heap, 256, TYPE_INT);  // Dead object
    }

    // Mark live objects
    for (int i = 0; i < 500; i++) {
        morph_v2_get_header(live_objects[i])->marked = 1;
    }

    // Benchmark compaction
    uint64_t start = get_time_us();
    gc_compact_old_generation(heap);
    uint64_t elapsed = get_time_us() - start;

    size_t objects_compacted = 500;
    size_t bytes_compacted = heap->bytes_compacted;

    printf("  Compacted %zu objects (%zu KB) in %lu us\n",
           objects_compacted, bytes_compacted / 1024, elapsed);
    printf("  Throughput: %.2f MB/s\n",
           (double)bytes_compacted / elapsed);

    gc_heap_destroy(heap);
    printf("\n");
}

void benchmark_heap_resize_performance(void) {
    printf(COLOR_YELLOW "[BENCHMARK] Heap Resize Performance" COLOR_RESET "\n");

    GCHeap* heap = gc_heap_create();

    size_t original_size = heap->young.size;

    // Benchmark grow
    uint64_t start = get_time_us();
    gc_resize_young_generation(heap, original_size * 2);
    uint64_t grow_time = get_time_us() - start;

    printf("  Young gen grow (2MB -> 4MB): %lu us\n", grow_time);

    // Benchmark shrink
    start = get_time_us();
    gc_resize_young_generation(heap, original_size);
    uint64_t shrink_time = get_time_us() - start;

    printf("  Young gen shrink (4MB -> 2MB): %lu us\n", shrink_time);

    gc_heap_destroy(heap);
    printf("\n");
}

//=============================================================================
// MAIN TEST RUNNER
//=============================================================================

int main(void) {
    printf("\n");
    printf("=======================================================\n");
    printf(" Morph Memory V2 - GC Optimization Tests (Week 12)\n");
    printf("=======================================================\n");
    printf("\n");

    // Precise Tracing Tests
    printf(COLOR_BLUE ">>> PRECISE TRACING TESTS <<<" COLOR_RESET "\n\n");
    test_type_descriptor_registration();
    test_precise_tracing_linked_list();
    test_precise_tracing_tree();
    test_precise_tracing_multiple_pointers();

    // Mark-Compact Tests
    printf(COLOR_BLUE ">>> MARK-COMPACT GC TESTS <<<" COLOR_RESET "\n\n");
    test_mark_compact_basic();
    test_mark_compact_pointer_fixup();
    test_mark_compact_fragmentation();

    // Heap Resizing Tests
    printf(COLOR_BLUE ">>> HEAP RESIZING TESTS <<<" COLOR_RESET "\n\n");
    test_heap_resize_young_grow();
    test_heap_resize_young_shrink();
    test_heap_resize_old_grow();
    test_heap_auto_resize();

    // Integration Tests
    printf(COLOR_BLUE ">>> INTEGRATION TESTS <<<" COLOR_RESET "\n\n");
    test_integration_precise_tracing_with_compaction();
    test_integration_full_gc_with_resizing();

    // Benchmarks
    printf(COLOR_BLUE ">>> PERFORMANCE BENCHMARKS <<<" COLOR_RESET "\n\n");
    benchmark_precise_tracing_overhead();
    benchmark_mark_compact_performance();
    benchmark_heap_resize_performance();

    // Summary
    printf("=======================================================\n");
    printf(" Test Results\n");
    printf("=======================================================\n");
    printf(COLOR_GREEN "  Passed: %d" COLOR_RESET "\n", tests_passed);
    if (tests_failed > 0) {
        printf(COLOR_RED "  Failed: %d" COLOR_RESET "\n", tests_failed);
    } else {
        printf("  Failed: 0\n");
    }
    printf("=======================================================\n");
    printf("\n");

    return tests_failed > 0 ? 1 : 0;
}
