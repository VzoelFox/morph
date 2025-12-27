/*
 * Morph Memory V2 - Bridge Layer
 * Week 5-6: N0 Integration
 *
 * Provides backward-compatible API for N0 compiler to use Memory V2
 * without changing existing codegen.
 */

#ifndef MORPH_MEM_V2_BRIDGE_H
#define MORPH_MEM_V2_BRIDGE_H

#include "morph.h"
#include "morph_mem_v2.h"

//=============================================================================
// BRIDGE CONFIGURATION
//=============================================================================

// Enable/disable Memory V2 (set to 1 to use V2, 0 for V1)
#ifndef USE_MEMORY_V2
#define USE_MEMORY_V2 1
#endif

// Memory mode selection
#ifndef MORPH_MEMORY_MODE
#define MORPH_MEMORY_MODE MORPH_MODE_COMPILER  // Default: COMPILER mode
#endif

//=============================================================================
// EXTENDED CONTEXT (wraps MorphContext with MorphContextV2)
//=============================================================================

typedef struct MorphContextBridge {
    MorphContext v1_ctx;        // Original V1 context (for compatibility)
    MorphContextV2* v2_ctx;     // Memory V2 context (if USE_MEMORY_V2 == 1)

    // Bridge state
    int using_v2;               // 1 if using V2, 0 if using V1
    size_t allocation_count;    // Total allocations (stats)
    size_t v1_alloc_count;      // V1 allocations (fallback)
    size_t v2_alloc_count;      // V2 allocations

    // Type mapping: V1 MorphTypeInfo* → V2 uint8_t type_id
    uint8_t next_type_id;
    MorphTypeInfo* type_map[128];  // Maps type_id → MorphTypeInfo*
} MorphContextBridge;

//=============================================================================
// BRIDGE API (Drop-in replacement for V1 functions)
//=============================================================================

// Initialization
MorphContextBridge* mph_bridge_init(void);
void mph_bridge_destroy(MorphContextBridge* bridge);

// Allocation (compatible with V1 mph_alloc signature)
void* mph_bridge_alloc(MorphContextBridge* bridge, MorphTypeInfo* type);

// GC control (compatible with V1 signatures)
void mph_bridge_gc_collect(MorphContextBridge* bridge);
void mph_bridge_gc_push_root(MorphContextBridge* bridge, void** ptr);
void mph_bridge_gc_pop_roots(MorphContextBridge* bridge, int count);

// Stats & diagnostics
void mph_bridge_print_stats(MorphContextBridge* bridge);
void mph_bridge_dump_stats(MorphContextBridge* bridge, const char* path);

//=============================================================================
// TYPE MAPPING HELPERS
//=============================================================================

// Register V1 type and get V2 type_id
uint8_t mph_bridge_register_type(MorphContextBridge* bridge, MorphTypeInfo* type);

// Get V1 type from V2 type_id
MorphTypeInfo* mph_bridge_get_type(MorphContextBridge* bridge, uint8_t type_id);

//=============================================================================
// COMPATIBILITY MACROS (for codegen)
//=============================================================================

#if USE_MEMORY_V2

// Redefine V1 functions to use bridge
#define mph_init_memory(ctx) /* No-op, use mph_bridge_init() instead */
#define mph_alloc(ctx, type) mph_bridge_alloc((MorphContextBridge*)(ctx), (type))
#define mph_gc_collect(ctx) mph_bridge_gc_collect((MorphContextBridge*)(ctx))
#define mph_gc_push_root(ctx, ptr) mph_bridge_gc_push_root((MorphContextBridge*)(ctx), (ptr))
#define mph_gc_pop_roots(ctx, count) mph_bridge_gc_pop_roots((MorphContextBridge*)(ctx), (count))

#else

// Use V1 implementation (no changes)
// Original mph_* functions from runtime.c.tpl

#endif

#endif // MORPH_MEM_V2_BRIDGE_H
