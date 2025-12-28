/*
 * Morph Memory V2 - Week 1 Tests
 * Test ObjectHeader, Config, and basic allocation
 */

#include "morph_mem_v2.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Test type IDs
#define TYPE_INT 1
#define TYPE_STRING 2
#define TYPE_ARRAY 3

// ANSI color codes
#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

void test_object_header_size(void) {
    printf("Test: ObjectHeader size...");

    // Verify header is exactly 16 bytes
    assert(sizeof(ObjectHeader) == 16);

    printf(GREEN " PASS\n" RESET);
}

void test_object_header_fields(void) {
    printf("Test: ObjectHeader bitfield layout...");

    ObjectHeader header = {0};

    // Test size field (24 bits)
    header.size = OBJECT_MAX_SIZE;
    assert(header.size == OBJECT_MAX_SIZE);

    header.size = 0;
    assert(header.size == 0);

    header.size = 1000;
    assert(header.size == 1000);

    // Test type_id field (7 bits)
    header.type_id = OBJECT_MAX_TYPE_ID;
    assert(header.type_id == OBJECT_MAX_TYPE_ID);

    header.type_id = 0;
    assert(header.type_id == 0);

    header.type_id = 42;
    assert(header.type_id == 42);

    // Test marked field (1 bit)
    header.marked = 0;
    assert(header.marked == 0);

    header.marked = 1;
    assert(header.marked == 1);

    // Test generation field
    header.generation = GEN_YOUNG;
    assert(header.generation == GEN_YOUNG);

    header.generation = GEN_OLD;
    assert(header.generation == GEN_OLD);

    header.generation = GEN_LARGE;
    assert(header.generation == GEN_LARGE);

    printf(GREEN " PASS\n" RESET);
}

void test_config_presets(void) {
    printf("Test: Config presets...");

    // Test COMPILER config
    assert(MORPH_CONFIG_COMPILER.mode == MORPH_MODE_COMPILER);
    assert(MORPH_CONFIG_COMPILER.gc_threshold == 0);
    assert(MORPH_CONFIG_COMPILER.enable_generational == 0);

    // Test RUNTIME config
    assert(MORPH_CONFIG_RUNTIME.mode == MORPH_MODE_RUNTIME);
    assert(MORPH_CONFIG_RUNTIME.gc_threshold > 0);
    assert(MORPH_CONFIG_RUNTIME.enable_generational == 1);

    // Test VM config
    assert(MORPH_CONFIG_VM.mode == MORPH_MODE_VM);
    assert(MORPH_CONFIG_VM.enable_compaction == 1);

    printf(GREEN " PASS\n" RESET);
}

void test_init_destroy(void) {
    printf("Test: Init and destroy...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    assert(ctx != NULL);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_basic_allocation(void) {
    printf("Test: Basic allocation...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);

    // Allocate small object
    void* ptr1 = morph_mem_alloc(ctx, 32, TYPE_INT);
    assert(ptr1 != NULL);

    // Verify header
    ObjectHeader* header1 = morph_v2_get_header(ptr1);
    assert(header1->size == 32);
    assert(header1->type_id == TYPE_INT);
    assert(header1->generation == GEN_YOUNG);

    // Allocate medium object
    void* ptr2 = morph_mem_alloc(ctx, 1024, TYPE_STRING);
    assert(ptr2 != NULL);

    ObjectHeader* header2 = morph_v2_get_header(ptr2);
    assert(header2->size == 1024);
    assert(header2->type_id == TYPE_STRING);

    // Allocate large object
    void* ptr3 = morph_mem_alloc(ctx, 64 * 1024, TYPE_ARRAY);
    assert(ptr3 != NULL);

    ObjectHeader* header3 = morph_v2_get_header(ptr3);
    assert(header3->size == 64 * 1024);
    assert(header3->type_id == TYPE_ARRAY);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_zeroed_allocation(void) {
    printf("Test: Zeroed allocation...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);

    // Allocate zeroed buffer
    void* ptr = morph_mem_alloc_zeroed(ctx, 256, TYPE_STRING);
    assert(ptr != NULL);

    // Verify all zeros
    unsigned char* buf = (unsigned char*)ptr;
    for (size_t i = 0; i < 256; i++) {
        assert(buf[i] == 0);
    }

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_type_registry(void) {
    printf("Test: Type registry...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);

    // Register types
    morph_mem_register_type(ctx, TYPE_INT, "Int");
    morph_mem_register_type(ctx, TYPE_STRING, "String");
    morph_mem_register_type(ctx, TYPE_ARRAY, "Array");

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_statistics(void) {
    printf("Test: Statistics tracking...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    morph_mem_register_type(ctx, TYPE_INT, "Int");

    // Allocate some objects
    void* p1 = morph_mem_alloc(ctx, 100, TYPE_INT);
    void* p2 = morph_mem_alloc(ctx, 200, TYPE_INT);
    void* p3 = morph_mem_alloc(ctx, 300, TYPE_INT);

    MorphMemStats stats = morph_mem_get_stats(ctx);

    // Verify stats
    assert(stats.object_count == 3);
    assert(stats.total_allocated == 600);
    assert(stats.current_live == 600);
    assert(stats.object_count_by_type[TYPE_INT] == 3);
    assert(stats.bytes_by_type[TYPE_INT] == 600);

    // Free one object
    morph_mem_free(ctx, p2);

    stats = morph_mem_get_stats(ctx);
    assert(stats.total_freed == 200);
    assert(stats.current_live == 400);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_gc_roots(void) {
    printf("Test: GC root stack...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_RUNTIME);

    void* obj1 = morph_mem_alloc(ctx, 64, TYPE_INT);
    void* obj2 = morph_mem_alloc(ctx, 128, TYPE_STRING);

    // Push roots
    morph_mem_gc_push_root(ctx, &obj1);
    morph_mem_gc_push_root(ctx, &obj2);

    // Pop roots
    morph_mem_gc_pop_roots(ctx, 2);

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

void test_ram_detection(void) {
    printf("Test: RAM detection...");

    size_t available = morph_mem_get_available_ram();
    assert(available > 0);

    printf(" (detected %lu MB)", available / (1024 * 1024));

    printf(GREEN " PASS\n" RESET);
}

void test_auto_config(void) {
    printf("Test: Auto config detection...");

    MorphMemConfig config = morph_mem_detect_config();

    // Should select reasonable mode
    assert(config.mode == MORPH_MODE_COMPILER ||
           config.mode == MORPH_MODE_RUNTIME);

    assert(config.heap_size_hint > 0);

    printf(GREEN " PASS\n" RESET);
}

void test_stats_dump(void) {
    printf("Test: Stats dump to file...");

    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    morph_mem_register_type(ctx, TYPE_INT, "Int");

    void* p1 = morph_mem_alloc(ctx, 1000, TYPE_INT);
    void* p2 = morph_mem_alloc(ctx, 2000, TYPE_INT);

    // Dump stats to file
    morph_mem_dump_stats(ctx, "/tmp/morph_mem_stats.json");

    morph_mem_destroy(ctx);

    printf(GREEN " PASS\n" RESET);
}

int main(void) {
    printf("\n=== Morph Memory V2 - Week 1 Tests ===\n\n");

    // Run all tests
    test_object_header_size();
    test_object_header_fields();
    test_config_presets();
    test_init_destroy();
    test_basic_allocation();
    test_zeroed_allocation();
    test_type_registry();
    test_statistics();
    test_gc_roots();
    test_ram_detection();
    test_auto_config();
    test_stats_dump();

    printf("\n" GREEN "=== All Tests Passed! ===" RESET "\n\n");

    // Demonstrate usage
    printf("=== Demonstration ===\n");
    MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER);
    morph_mem_register_type(ctx, TYPE_INT, "Int");
    morph_mem_register_type(ctx, TYPE_STRING, "String");
    morph_mem_register_type(ctx, TYPE_ARRAY, "Array");

    // Allocate various objects
    for (int i = 0; i < 100; i++) {
        morph_mem_alloc(ctx, 32, TYPE_INT);
        morph_mem_alloc(ctx, 64, TYPE_STRING);
        morph_mem_alloc(ctx, 128, TYPE_ARRAY);
    }

    // Print stats
    morph_mem_print_stats(ctx);

    morph_mem_destroy(ctx);

    return 0;
}
