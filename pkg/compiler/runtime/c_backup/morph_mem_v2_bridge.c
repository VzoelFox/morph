/*
 * Morph Memory V2 - Bridge Implementation
 * Week 5-6: N0 Integration
 *
 * Bridges V1 API (MorphContext, mph_alloc) to V2 API (MorphContextV2)
 */

#include "morph_mem_v2_bridge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//=============================================================================
// INITIALIZATION
//=============================================================================

MorphContextBridge* mph_bridge_init(void) {
    MorphContextBridge* bridge = (MorphContextBridge*)malloc(sizeof(MorphContextBridge));
    if (!bridge) {
        fprintf(stderr, "FATAL: Failed to allocate MorphContextBridge\n");
        abort();
    }

    memset(bridge, 0, sizeof(MorphContextBridge));

#if USE_MEMORY_V2
    // Initialize Memory V2
    MorphMemConfig config;

    // Select config based on MORPH_MEMORY_MODE
    switch (MORPH_MEMORY_MODE) {
        case MORPH_MODE_COMPILER:
            config = MORPH_CONFIG_COMPILER;
            break;
        case MORPH_MODE_RUNTIME:
            config = MORPH_CONFIG_RUNTIME;
            break;
        case MORPH_MODE_VM:
            config = MORPH_CONFIG_VM;
            break;
        case MORPH_MODE_SERVER:
            config = MORPH_CONFIG_SERVER;
            break;
        default:
            config = morph_mem_detect_config();  // Auto-detect
            break;
    }

    // Enable debug for development
    config.enable_debug = 1;
    config.enable_metrics = 1;

    bridge->v2_ctx = morph_mem_init(config);
    bridge->using_v2 = 1;

    printf("[Bridge] Memory V2 initialized - Mode: %d\n", config.mode);
#else
    // Initialize Memory V1 (original implementation)
    mph_init_memory(&bridge->v1_ctx);
    bridge->using_v2 = 0;

    printf("[Bridge] Memory V1 initialized (fallback)\n");
#endif

    bridge->next_type_id = 1;  // Start from 1 (0 reserved)

    return bridge;
}

void mph_bridge_destroy(MorphContextBridge* bridge) {
    if (!bridge) return;

#if USE_MEMORY_V2
    if (bridge->v2_ctx) {
        printf("\n[Bridge] Final Statistics:\n");
        printf("  V2 allocations: %zu\n", bridge->v2_alloc_count);
        printf("  Total allocations: %zu\n", bridge->allocation_count);

        morph_mem_print_stats(bridge->v2_ctx);
        morph_mem_destroy(bridge->v2_ctx);
    }
#else
    // Cleanup V1 context (if needed)
    // mph_cleanup_memory(&bridge->v1_ctx);
#endif

    free(bridge);
}

//=============================================================================
// TYPE MAPPING
//=============================================================================

uint8_t mph_bridge_register_type(MorphContextBridge* bridge, MorphTypeInfo* type) {
    if (!type) return 0;

    // Check if already registered
    for (uint8_t i = 1; i < bridge->next_type_id; i++) {
        if (bridge->type_map[i] == type) {
            return i;  // Already registered
        }
    }

    // Register new type
    uint8_t type_id = bridge->next_type_id++;
    if (type_id > OBJECT_MAX_TYPE_ID) {
        fprintf(stderr, "ERROR: Type ID overflow (max %d)\n", OBJECT_MAX_TYPE_ID);
        return 0;
    }

    bridge->type_map[type_id] = type;

#if USE_MEMORY_V2
    // Register in V2
    morph_mem_register_type(bridge->v2_ctx, type_id, type->name);
#endif

    return type_id;
}

MorphTypeInfo* mph_bridge_get_type(MorphContextBridge* bridge, uint8_t type_id) {
    if (type_id == 0 || type_id >= bridge->next_type_id) {
        return NULL;
    }
    return bridge->type_map[type_id];
}

//=============================================================================
// ALLOCATION
//=============================================================================

void* mph_bridge_alloc(MorphContextBridge* bridge, MorphTypeInfo* type) {
    if (!bridge || !type) return NULL;

    bridge->allocation_count++;

#if USE_MEMORY_V2
    // Use Memory V2
    bridge->v2_alloc_count++;

    // Get or register type ID
    uint8_t type_id = mph_bridge_register_type(bridge, type);

    // Allocate using V2
    void* payload = morph_mem_alloc(bridge->v2_ctx, type->size, type_id);
    if (!payload) {
        fprintf(stderr, "ERROR: V2 allocation failed for type %s (%zu bytes)\n",
                type->name, type->size);
        return NULL;
    }

    // Zero-initialize payload (V1 compatibility)
    memset(payload, 0, type->size);

    return payload;

#else
    // Use Memory V1 (fallback)
    bridge->v1_alloc_count++;
    return mph_alloc(&bridge->v1_ctx, type);
#endif
}

//=============================================================================
// GC CONTROL
//=============================================================================

void mph_bridge_gc_collect(MorphContextBridge* bridge) {
    if (!bridge) return;

#if USE_MEMORY_V2
    // Memory V2 GC
    if (bridge->v2_ctx->config.mode == MORPH_MODE_COMPILER) {
        // COMPILER mode: No GC (arena-based)
        // Do nothing - memory freed at end
    } else {
        // RUNTIME/VM/SERVER mode: Trigger GC
        morph_mem_gc_collect(bridge->v2_ctx);
    }
#else
    // Memory V1 GC
    mph_gc_collect(&bridge->v1_ctx);
#endif
}

void mph_bridge_gc_push_root(MorphContextBridge* bridge, void** ptr) {
    if (!bridge || !ptr) return;

#if USE_MEMORY_V2
    morph_mem_gc_push_root(bridge->v2_ctx, ptr);
#else
    // V1 root push
    StackRoot* root = (StackRoot*)malloc(sizeof(StackRoot));
    root->ptr = ptr;
    root->next = bridge->v1_ctx.stack_top;
    bridge->v1_ctx.stack_top = root;
#endif
}

void mph_bridge_gc_pop_roots(MorphContextBridge* bridge, int count) {
    if (!bridge) return;

#if USE_MEMORY_V2
    morph_mem_gc_pop_roots(bridge->v2_ctx, count);
#else
    // V1 root pop
    for (int i = 0; i < count && bridge->v1_ctx.stack_top; i++) {
        StackRoot* old = bridge->v1_ctx.stack_top;
        bridge->v1_ctx.stack_top = old->next;
        free(old);
    }
#endif
}

//=============================================================================
// STATS & DIAGNOSTICS
//=============================================================================

void mph_bridge_print_stats(MorphContextBridge* bridge) {
    if (!bridge) return;

    printf("\n=== Bridge Statistics ===\n");
    printf("Memory backend: %s\n", bridge->using_v2 ? "V2" : "V1");
    printf("Total allocations: %zu\n", bridge->allocation_count);
    printf("V2 allocations: %zu\n", bridge->v2_alloc_count);
    printf("V1 allocations: %zu\n", bridge->v1_alloc_count);

#if USE_MEMORY_V2
    if (bridge->v2_ctx) {
        morph_mem_print_stats(bridge->v2_ctx);
    }
#endif
}

void mph_bridge_dump_stats(MorphContextBridge* bridge, const char* path) {
    if (!bridge) return;

#if USE_MEMORY_V2
    if (bridge->v2_ctx) {
        morph_mem_dump_stats(bridge->v2_ctx, path);
    }
#endif
}
