#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "morph.h"

// --- Z-Alloc: Paged Arena & Zombie Log ---

#define FLAG_MARKED 0x01
#define FLAG_SWAPPED 0x02
#define PAGE_SIZE 4096
#define PAGE_HEADER_SIZE sizeof(MphPage)

// Forward Declarations
void mph_gc_mark(MorphContext* ctx, void* obj);
void mph_gc_mark_array(MorphContext* ctx, void* obj);
void mph_gc_mark_map(MorphContext* ctx, void* obj);

// Simple TypeInfos
MorphTypeInfo mph_ti_string = { "string", sizeof(MorphString), 0, NULL, NULL };
size_t string_ptr_offsets[] = { offsetof(MorphString, data) };
MorphTypeInfo mph_ti_string_real = { "string", sizeof(MorphString), 1, string_ptr_offsets, NULL };
MorphTypeInfo mph_ti_raw = { "raw", 0, 0, NULL, NULL };

// Container TypeInfos
MorphTypeInfo mph_ti_array = { "array", sizeof(MorphArray), 0, NULL, mph_gc_mark_array };
MorphTypeInfo mph_ti_map = { "map", sizeof(MorphMap), 0, NULL, mph_gc_mark_map };

// Closure TypeInfo
size_t closure_ptr_offsets[] = { offsetof(MorphClosure, env) };
MorphTypeInfo mph_ti_closure = { "closure", sizeof(MorphClosure), 1, closure_ptr_offsets, NULL };

// --- Utils ---
uint64_t mph_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// --- Page Management ---

MphPage* mph_page_new(MorphContext* ctx) {
    MphPage* page = (MphPage*)malloc(sizeof(MphPage));
    // Allocate 4KB aligned memory
    // mmap is easiest for page alignment
    page->start_addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (page->start_addr == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    page->used_offset = 0;
    page->flags = 0;
    page->last_access = mph_time_ms();
    page->swap_id = 0;
    page->live_bytes = 0;
    page->size = PAGE_SIZE;
    page->free_list = NULL;

    pthread_mutex_lock(&ctx->page_lock);
    page->next = ctx->page_head;
    ctx->page_head = page;
    pthread_mutex_unlock(&ctx->page_lock);

    return page;
}

void mph_page_free(MphPage* page) {
    if (page->start_addr && !(page->flags & FLAG_SWAPPED)) {
        munmap(page->start_addr, page->size);
    }
    // Unlink from list? (Simplified: we don't unlink for now, just leak struct or sweep list later)
    free(page);
}

// --- Memory ---

void mph_init_memory(MorphContext* ctx) {
    ctx->heap_head = NULL;
    ctx->allocated_bytes = 0;
    ctx->next_gc_threshold = GC_MIN_THRESHOLD;
    ctx->stack_top = NULL;
    ctx->daemon_running = 0;
    ctx->page_head = NULL;
    ctx->current_alloc_page = NULL;
    ctx->free_list = NULL;

    // Initialize Mark Stack (Iterative GC)
    ctx->mark_stack.head = NULL;
    ctx->mark_stack.current = NULL;
    ctx->mark_stack.count = 0;

    pthread_mutex_init(&ctx->memory_lock, NULL);
    pthread_mutex_init(&ctx->page_lock, NULL);

    // Create directories
    mkdir(".morph.vz", 0755);
    mkdir(".morph.vz/swap", 0755);
    // Create .z file
    FILE* z = fopen(".morph.vz/.z", "a");
    if (z) fclose(z);

    mph_start_daemon(ctx);
}

void mph_destroy_memory(MorphContext* ctx) {
    mph_stop_daemon(ctx);
    MarkStackBlock* block = ctx->mark_stack.head;
    while (block) {
        MarkStackBlock* next = block->next;
        free(block);
        block = next;
    }
    ctx->mark_stack.head = NULL;
    ctx->mark_stack.current = NULL;
    ctx->mark_stack.count = 0;
    ctx->free_list = NULL;
    pthread_mutex_lock(&ctx->page_lock);
    MphPage* page = ctx->page_head;
    while (page) {
        MphPage* next = page->next;
        if (page->flags & FLAG_SWAPPED) {
            char path[256];
            sprintf(path, ".morph.vz/swap/page_%lu.bin", page->swap_id);
            unlink(path);
        }
        mph_page_free(page);
        page = next;
    }
    ctx->page_head = NULL;
    ctx->current_alloc_page = NULL;
    pthread_mutex_unlock(&ctx->page_lock);
    pthread_mutex_destroy(&ctx->page_lock);
    pthread_mutex_destroy(&ctx->memory_lock);
}

// Helper to get header from payload
ObjectHeader* mph_get_header(void* obj) {
    return ((ObjectHeader*)obj) - 1;
}

// Shadow Stack
void mph_gc_push_root(MorphContext* ctx, void** ptr) {
    StackRoot* node = (StackRoot*)malloc(sizeof(StackRoot));
    node->ptr = ptr;
    node->next = ctx->stack_top;
    ctx->stack_top = node;
}

void mph_gc_pop_roots(MorphContext* ctx, int count) {
    for (int i = 0; i < count; i++) {
        if (ctx->stack_top) {
            StackRoot* temp = ctx->stack_top;
            ctx->stack_top = ctx->stack_top->next;
            free(temp);
        }
    }
}

// Swap Logic (Page Based)
void mph_page_swap_out(MphPage* page) {
    if (page->flags & FLAG_SWAPPED) return;

    page->swap_id = mph_time_ms() + (uint64_t)page;
    char path[256];
    sprintf(path, ".morph.vz/swap/page_%lu.bin", page->swap_id);

    FILE* f = fopen(path, "wb");
    if (f) {
        fwrite(page->start_addr, page->size, 1, f);
        fclose(f);

        // Decommit memory but keep address reserved
        void* res = mmap(page->start_addr, page->size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        if (res == MAP_FAILED) perror("swap out mmap failed");

        page->flags |= FLAG_SWAPPED;
    }
}

void mph_page_swap_in(MphPage* page) {
    if (!(page->flags & FLAG_SWAPPED)) return;

    // Restore memory at fixed address
    void* res = mmap(page->start_addr, page->size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (res == MAP_FAILED) {
        perror("swap in mmap failed");
        exit(1);
    }

    char path[256];
    sprintf(path, ".morph.vz/swap/page_%lu.bin", page->swap_id);
    FILE* f = fopen(path, "rb");
    if (f) {
        fread(page->start_addr, page->size, 1, f);
        fclose(f);
        unlink(path);
    }

    page->flags &= ~FLAG_SWAPPED;
    page->last_access = mph_time_ms();
}

// Find Page for Address
MphPage* mph_find_page(MorphContext* ctx, void* addr) {
    uint64_t addr_val = (uint64_t)addr;

    pthread_mutex_lock(&ctx->page_lock);
    MphPage* cur = ctx->page_head;
    while (cur) {
        uint64_t start = (uint64_t)cur->start_addr;
        uint64_t end = start + cur->size;
        if (addr_val >= start && addr_val < end) {
            pthread_mutex_unlock(&ctx->page_lock);
            return cur;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&ctx->page_lock);
    return NULL;
}

void mph_swap_in(MorphContext* ctx, void* obj) {
    if (!obj) return;
    ObjectHeader* header = mph_get_header(obj);
    MphPage* page = header ? header->page : NULL;
    if (!page) {
        page = mph_find_page(ctx, obj);
    }
    if (page) {
        if (page->flags & FLAG_SWAPPED) {
            mph_page_swap_in(page);
        } else {
            page->last_access = mph_time_ms();
        }
    }
}

static MarkStackBlock* mph_mark_stack_block_new(void) {
    MarkStackBlock* block = (MarkStackBlock*)malloc(sizeof(MarkStackBlock));
    if (!block) { perror("malloc mark stack block"); exit(1); }
    block->count = 0;
    block->next = NULL;
    block->prev = NULL;
    return block;
}

// Helper: Push to Mark Stack (Block Growth)
void mph_mark_stack_push(MorphContext* ctx, void* obj) {
    if (!obj) return;
    MarkStack* stack = &ctx->mark_stack;
    if (!stack->current) {
        if (!stack->head) {
            stack->head = mph_mark_stack_block_new();
        }
        stack->current = stack->head;
        stack->current->count = 0;
    }

    if (stack->current->count >= MARK_STACK_BLOCK_SIZE) {
        if (stack->current->next) {
            stack->current = stack->current->next;
            stack->current->count = 0;
        } else {
            MarkStackBlock* block = mph_mark_stack_block_new();
            block->prev = stack->current;
            stack->current->next = block;
            stack->current = block;
        }
    }

    stack->current->items[stack->current->count++] = obj;
    stack->count++;
}

static void* mph_mark_stack_pop(MorphContext* ctx) {
    MarkStack* stack = &ctx->mark_stack;
    if (stack->count == 0) return NULL;
    MarkStackBlock* current = stack->current;
    while (current && current->count == 0) {
        current = current->prev;
    }
    stack->current = current;
    if (!current) return NULL;
    stack->count--;
    return current->items[--current->count];
}

// Iterative Mark Function: Just queues the object if not marked
void mph_gc_mark(MorphContext* ctx, void* obj) {
    if (!obj) return;

    // Check marking BEFORE pushing to stack to avoid duplicates
    ObjectHeader* header = mph_get_header(obj);
    if (header->flags & FLAG_MARKED) return; // Already marked

    mph_swap_in(ctx, obj);

    header->flags |= FLAG_MARKED;

    // Track live bytes for Page Recycling
    MphPage* page = header->page ? header->page : mph_find_page(ctx, obj);
    if (page) {
        page->live_bytes += (header->size + sizeof(ObjectHeader));
    }

    // Queue for processing children
    mph_mark_stack_push(ctx, obj);
}

// Callback implementations (push children)
void mph_gc_mark_array(MorphContext* ctx, void* obj) {
    MorphArray* arr = (MorphArray*)obj;
    if (arr->data) mph_gc_mark(ctx, arr->data);
    if (arr->elements_are_pointers) {
        mph_swap_in(ctx, arr->data); // Ensure data is in RAM
        void** elements = (void**)arr->data;
        for (size_t i = 0; i < arr->length; i++) {
            if (elements[i]) mph_gc_mark(ctx, elements[i]);
        }
    }
}

void mph_gc_mark_map(MorphContext* ctx, void* obj) {
    MorphMap* map = (MorphMap*)obj;
    if (map->entries) mph_gc_mark(ctx, map->entries);

    mph_swap_in(ctx, map->entries); // Ensure entries are in RAM
    for (size_t i = 0; i < map->capacity; i++) {
        MorphMapEntry* e = &map->entries[i];
        if (e->occupied) {
            if (map->key_kind == MPH_KEY_STRING || map->key_kind == MPH_KEY_PTR) {
                if (e->key) mph_gc_mark(ctx, e->key);
            }
            if (map->values_are_pointers) {
                if (e->value) mph_gc_mark(ctx, e->value);
            }
        }
    }
}

// Process the Mark Stack (The Loop)
void mph_gc_process_mark_stack(MorphContext* ctx) {
    while (ctx->mark_stack.count > 0) {
        void* obj = mph_mark_stack_pop(ctx);
        if (!obj) continue;

        ObjectHeader* header = mph_get_header(obj);
        MorphTypeInfo* type = header->type;
        if (!type) continue;

        if (type->mark_fn) {
            type->mark_fn(ctx, obj);
        } else {
            char* payload = (char*)obj;
            for (int i = 0; i < type->num_pointers; i++) {
                size_t offset = type->pointer_offsets[i];
                void** child_ptr = (void**)(payload + offset);
                if (*child_ptr) {
                    mph_gc_mark(ctx, *child_ptr);
                }
            }
        }
    }
}

// Zombie Logging
void mph_log_zombie(ObjectHeader* obj) {
    FILE* f = fopen(".morph.vz/.z", "ab");
    if (!f) return;

    // Log Format: [Time] [Addr] [Type] [Size]
    uint64_t now = mph_time_ms();
    const char* type_name = obj->type ? obj->type->name : "raw";
    size_t size = obj->size;

    fprintf(f, "[%lu] DEAD: %p Type=%s Size=%lu\n", now, (void*)(obj+1), type_name, size);
    fclose(f);
}

void mph_gc_sweep(MorphContext* ctx) {
    ObjectHeader** curr_ptr = &ctx->heap_head;
    while (*curr_ptr) {
        ObjectHeader* curr = *curr_ptr;
        if (curr->flags & FLAG_MARKED) {
            curr->flags &= ~FLAG_MARKED;
            curr_ptr = &curr->next;
        } else {
            *curr_ptr = curr->next;
            // ZOMBIE LOGGING before free
            mph_log_zombie(curr);

            ctx->allocated_bytes -= curr->size;
            curr->flags = 0;
            curr->free_prev = NULL;
            curr->free_next = ctx->free_list;
            if (ctx->free_list) {
                ctx->free_list->free_prev = curr;
            }
            ctx->free_list = curr;
            curr->page_free_prev = NULL;
            curr->page_free_next = curr->page->free_list;
            if (curr->page->free_list) {
                curr->page->free_list->page_free_prev = curr;
            }
            curr->page->free_list = curr;
        }
    }

    // Page Recycling
    pthread_mutex_lock(&ctx->page_lock);
    MphPage** p_ptr = &ctx->page_head;
    while (*p_ptr) {
        MphPage* p = *p_ptr;
        // Do not free if it's the current allocation page (even if empty/0 live bytes because it's filling up)
        // Actually, current_alloc_page might be full and have 0 live bytes. Then we free it.
        // But if it's the one we are WRITING to, we shouldn't unmap it under our feet.
        // Simplified check: If p == current_alloc_page, keep it.

        // Also skip pages that are swapped out? No, if swapped out and live_bytes=0, we can kill it?
        // But mark requires swap in. So if it stayed swapped out, it wasn't marked.
        // If it wasn't marked, live_bytes is 0.
        // So we can free swapped out pages too!

        if (p != ctx->current_alloc_page && p->live_bytes == 0) {
            ObjectHeader* free_hdr = p->free_list;
            while (free_hdr) {
                ObjectHeader* next_free = free_hdr->page_free_next;
                if (free_hdr->free_prev) {
                    free_hdr->free_prev->free_next = free_hdr->free_next;
                } else if (ctx->free_list == free_hdr) {
                    ctx->free_list = free_hdr->free_next;
                }
                if (free_hdr->free_next) {
                    free_hdr->free_next->free_prev = free_hdr->free_prev;
                }
                free_hdr = next_free;
            }
            p->free_list = NULL;
            *p_ptr = p->next;
            if (p->flags & FLAG_SWAPPED) {
                 char path[256];
                 sprintf(path, ".morph.vz/swap/page_%lu.bin", p->swap_id);
                 unlink(path);
            }
            mph_page_free(p);
        } else {
            p_ptr = &p->next;
        }
    }
    pthread_mutex_unlock(&ctx->page_lock);
}

void mph_gc_collect(MorphContext* ctx) {
    pthread_mutex_lock(&ctx->memory_lock);

    // Reset Live Bytes counters on all pages
    pthread_mutex_lock(&ctx->page_lock);
    MphPage* p = ctx->page_head;
    while (p) {
        p->live_bytes = 0;
        p = p->next;
    }
    pthread_mutex_unlock(&ctx->page_lock);

    StackRoot* root = ctx->stack_top;
    while (root) {
        if (root->ptr && *root->ptr) mph_gc_mark(ctx, *root->ptr);
        root = root->next;
    }

    // Drain the mark stack
    mph_gc_process_mark_stack(ctx);

    mph_gc_sweep(ctx);
    pthread_mutex_unlock(&ctx->memory_lock);
}

// Daemon
void* mph_daemon_loop(void* arg) {
    MorphContext* ctx = (MorphContext*)arg;
    int ticks = 0;
    while (ctx->daemon_running) {
        usleep(DAEMON_SLEEP_MS * 1000);
        ticks++;

        pthread_mutex_lock(&ctx->page_lock);
        uint64_t now = mph_time_ms();
        MphPage* cur = ctx->page_head;
        while (cur) {
            if (!(cur->flags & FLAG_SWAPPED) && (now - cur->last_access > SWAP_AGE_THRESHOLD_SEC * 1000)) {
                mph_page_swap_out(cur);
            }
            cur = cur->next;
        }
        pthread_mutex_unlock(&ctx->page_lock);

        // Idle GC Trigger
        // If no allocation for 2 seconds and we have > 1MB allocated, trigger GC
        if (ctx->allocated_bytes > 1024 * 1024 && (now - ctx->last_alloc_time > 2000)) {
             // Reset timer to avoid loop
             ctx->last_alloc_time = now;
             mph_gc_collect(ctx);
        }
    }
    return NULL;
}

void mph_start_daemon(MorphContext* ctx) {
    ctx->daemon_running = 1;
    pthread_create(&ctx->daemon_thread, NULL, mph_daemon_loop, ctx);
}

void mph_stop_daemon(MorphContext* ctx) {
    ctx->daemon_running = 0;
    pthread_join(ctx->daemon_thread, NULL);
}

// Allocation (Arena Style)

void* mph_alloc(MorphContext* ctx, size_t size, MorphTypeInfo* type_info) {
    if (ctx->allocated_bytes > ctx->next_gc_threshold) {
        mph_gc_collect(ctx);
        size_t base_threshold = GC_THRESHOLD;
        if (ctx->allocated_bytes < GC_THRESHOLD) {
            base_threshold = GC_MIN_THRESHOLD;
        }
        ctx->next_gc_threshold = ctx->allocated_bytes + base_threshold;
    }

    // Update last alloc time for Idle detection
    ctx->last_alloc_time = mph_time_ms();

    size_t total_size = sizeof(ObjectHeader) + size;
    // Align to 8 bytes
    total_size = (total_size + 7) & ~7;

    pthread_mutex_lock(&ctx->memory_lock);

    // Reuse freed objects (exact size match)
    ObjectHeader* free_hdr = ctx->free_list;
    while (free_hdr) {
        if (free_hdr->size == size) {
            if (free_hdr->free_prev) {
                free_hdr->free_prev->free_next = free_hdr->free_next;
            } else {
                ctx->free_list = free_hdr->free_next;
            }
            if (free_hdr->free_next) {
                free_hdr->free_next->free_prev = free_hdr->free_prev;
            }
            if (free_hdr->page_free_prev) {
                free_hdr->page_free_prev->page_free_next = free_hdr->page_free_next;
            } else if (free_hdr->page && free_hdr->page->free_list == free_hdr) {
                free_hdr->page->free_list = free_hdr->page_free_next;
            }
            if (free_hdr->page_free_next) {
                free_hdr->page_free_next->page_free_prev = free_hdr->page_free_prev;
            }
            free_hdr->type = type_info;
            free_hdr->flags = 0;
            free_hdr->size = size;
            free_hdr->next = ctx->heap_head;
            ctx->heap_head = free_hdr;
            ctx->allocated_bytes += size;
            pthread_mutex_unlock(&ctx->memory_lock);
            return (void*)(free_hdr + 1);
        }
        free_hdr = free_hdr->free_next;
    }

    // Large Object Handling
    if (total_size > PAGE_SIZE) {
        // Allocate standalone multi-page block
        size_t required_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
        size_t alloc_size = required_pages * PAGE_SIZE;

        MphPage* page = (MphPage*)malloc(sizeof(MphPage));
        page->start_addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (page->start_addr == MAP_FAILED) { perror("mmap large failed"); exit(1); }
        page->used_offset = total_size;
        page->flags = 0;
        page->last_access = mph_time_ms();
        page->swap_id = 0;
        page->live_bytes = 0;
        page->size = alloc_size;
        page->free_list = NULL;

        pthread_mutex_lock(&ctx->page_lock);
        page->next = ctx->page_head;
        ctx->page_head = page;
        pthread_mutex_unlock(&ctx->page_lock);

        ObjectHeader* header = (ObjectHeader*)page->start_addr;
        memset(header, 0, total_size);
        header->type = type_info;
        header->flags = 0;
        header->size = size;
        header->page = page;

        header->next = ctx->heap_head;
        ctx->heap_head = header;
        ctx->allocated_bytes += size;

        pthread_mutex_unlock(&ctx->memory_lock);
        return (void*)(header + 1);
    }

    if (!ctx->current_alloc_page || (ctx->current_alloc_page->used_offset + total_size > PAGE_SIZE)) {
        // New Page
        ctx->current_alloc_page = mph_page_new(ctx);
    }

    void* addr = (char*)ctx->current_alloc_page->start_addr + ctx->current_alloc_page->used_offset;
    ctx->current_alloc_page->used_offset += total_size;

    ObjectHeader* header = (ObjectHeader*)addr;
    memset(header, 0, total_size);
    header->type = type_info;
    header->flags = 0;
    header->size = size;
    header->page = ctx->current_alloc_page;

    // Link to heap list for GC sweeping
    header->next = ctx->heap_head;
    ctx->heap_head = header;
    ctx->allocated_bytes += size;

    pthread_mutex_unlock(&ctx->memory_lock);

    return (void*)(header + 1);
}

// --- Strings ---

MorphString* mph_string_new(MorphContext* ctx, const char* literal) {
    size_t len = strlen(literal);
    MorphString* str = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    mph_gc_push_root(ctx, (void**)&str);
    str->length = len;

    char* data = (char*)mph_alloc(ctx, len + 1, &mph_ti_raw);
    mph_gc_push_root(ctx, (void**)&data);
    memcpy(data, literal, len);
    data[len] = '\0';
    str->data = data;
    mph_gc_pop_roots(ctx, 2);
    return str;
}

MorphString* mph_string_concat(MorphContext* ctx, MorphString* a, MorphString* b) {
    int roots = 0;
    if (a) { mph_gc_push_root(ctx, (void**)&a); roots++; }
    if (b) { mph_gc_push_root(ctx, (void**)&b); roots++; }
    mph_swap_in(ctx, a);
    mph_swap_in(ctx, a->data);
    mph_swap_in(ctx, b);
    mph_swap_in(ctx, b->data);
    size_t len = a->length + b->length;
    MorphString* str = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    mph_gc_push_root(ctx, (void**)&str);
    str->length = len;

    char* data = (char*)mph_alloc(ctx, len + 1, &mph_ti_raw);
    mph_gc_push_root(ctx, (void**)&data);
    memcpy(data, a->data, a->length);
    memcpy(data + a->length, b->data, b->length);
    data[len] = '\0';
    str->data = data;
    mph_gc_pop_roots(ctx, roots + 2);
    return str;
}

mph_bool mph_string_eq(MorphContext* ctx, MorphString* a, MorphString* b) {
    mph_swap_in(ctx, a);
    mph_swap_in(ctx, a->data);
    mph_swap_in(ctx, b);
    mph_swap_in(ctx, b->data);
    if (a->length != b->length) return 0;
    return memcmp(a->data, b->data, a->length) == 0;
}

mph_int mph_string_len(MorphContext* ctx, MorphString* s) {
    mph_swap_in(ctx, s);
    return (mph_int)s->length;
}

mph_int mph_string_index(MorphContext* ctx, MorphString* s, MorphString* sub) {
    mph_swap_in(ctx, s); mph_swap_in(ctx, s->data);
    mph_swap_in(ctx, sub); mph_swap_in(ctx, sub->data);

    char* p = strstr(s->data, sub->data);
    if (!p) return -1;
    return (mph_int)(p - s->data);
}

MorphString* mph_string_trim(MorphContext* ctx, MorphString* s, MorphString* cut) {
    int roots = 0;
    if (s) { mph_gc_push_root(ctx, (void**)&s); roots++; }
    if (cut) { mph_gc_push_root(ctx, (void**)&cut); roots++; }
    mph_swap_in(ctx, s); mph_swap_in(ctx, s->data);
    mph_swap_in(ctx, cut); mph_swap_in(ctx, cut->data);

    char* start = s->data;
    char* end = s->data + s->length - 1;

    while (start <= end && strchr(cut->data, *start)) start++;
    while (end > start && strchr(cut->data, *end)) end--;

    size_t new_len = (start > end) ? 0 : (size_t)(end - start + 1);

    MorphString* ret = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    mph_gc_push_root(ctx, (void**)&ret);
    ret->length = new_len;
    char* data = (char*)mph_alloc(ctx, new_len + 1, &mph_ti_raw);
    mph_gc_push_root(ctx, (void**)&data);
    if (new_len > 0) memcpy(data, start, new_len);
    data[new_len] = 0;
    ret->data = data;
    mph_gc_pop_roots(ctx, roots + 2);
    return ret;
}

MorphArray* mph_string_split(MorphContext* ctx, MorphString* s, MorphString* sep) {
    int roots = 0;
    if (s) { mph_gc_push_root(ctx, (void**)&s); roots++; }
    if (sep) { mph_gc_push_root(ctx, (void**)&sep); roots++; }
    mph_swap_in(ctx, s); mph_swap_in(ctx, s->data);
    mph_swap_in(ctx, sep); mph_swap_in(ctx, sep->data);

    // First pass: count parts
    size_t count = 0;
    char* p = s->data;
    size_t sep_len = sep->length;
    if (sep_len == 0) {
        count = s->length;
    } else {
        count = 1;
        while ((p = strstr(p, sep->data)) != NULL) {
            count++;
            p += sep_len;
        }
    }

    MorphArray* arr = mph_array_new(ctx, count, sizeof(MorphString*), 1);
    mph_gc_push_root(ctx, (void**)&arr);
    MorphString** elements = (MorphString**)arr->data;

    // Second pass: fill
    p = s->data;
    size_t idx = 0;
    if (sep_len == 0) {
        for (size_t i = 0; i < s->length; i++) {
            char buf[2] = { s->data[i], 0 };
            elements[i] = mph_string_new(ctx, buf);
        }
    } else {
        char* start = s->data;
        char* found;
        while ((found = strstr(start, sep->data)) != NULL) {
             size_t len = found - start;
             MorphString* item = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
             mph_gc_push_root(ctx, (void**)&item);
             item->length = len;
             item->data = (char*)mph_alloc(ctx, len + 1, &mph_ti_raw);
             mph_gc_push_root(ctx, (void**)&item->data);
             memcpy(item->data, start, len);
             item->data[len] = 0;

             elements[idx++] = item;
             mph_gc_pop_roots(ctx, 2);
             start = found + sep_len;
        }
        // Last part
        size_t len = strlen(start);
        MorphString* item = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
        mph_gc_push_root(ctx, (void**)&item);
        item->length = len;
        item->data = (char*)mph_alloc(ctx, len + 1, &mph_ti_raw);
        mph_gc_push_root(ctx, (void**)&item->data);
        memcpy(item->data, start, len);
        item->data[len] = 0;
        elements[idx++] = item;
        mph_gc_pop_roots(ctx, 2);
    }
    mph_gc_pop_roots(ctx, roots + 1);
    return arr;
}

MorphString* mph_string_substring(MorphContext* ctx, MorphString* s, mph_int start, mph_int end) {
    if (!s) {
        return mph_string_new(ctx, "");
    }
    int roots = 0;
    mph_gc_push_root(ctx, (void**)&s); roots++;
    mph_swap_in(ctx, s);
    if (s->data) mph_swap_in(ctx, s->data);
    if (start < 0) start = 0;
    if (end > s->length) end = s->length;
    if (start >= end || !s->data) {
        MorphString* empty = mph_string_new(ctx, "");
        mph_gc_pop_roots(ctx, roots);
        return empty;
    }

    size_t len = end - start;
    MorphString* ret = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    mph_gc_push_root(ctx, (void**)&ret);
    ret->length = len;
    ret->data = (char*)mph_alloc(ctx, len + 1, &mph_ti_raw);
    mph_gc_push_root(ctx, (void**)&ret->data);
    memcpy(ret->data, s->data + start, len);
    ret->data[len] = 0;
    mph_gc_pop_roots(ctx, roots + 2);
    return ret;
}

mph_int mph_string_Length(MorphContext* ctx, void* _env, MorphString* s) {
    return mph_string_len(ctx, s);
}

MorphString* mph_string_Concat(MorphContext* ctx, void* _env, MorphString* a, MorphString* b) {
    return mph_string_concat(ctx, a, b);
}

// --- Arrays ---

MorphArray* mph_array_new(MorphContext* ctx, size_t capacity, size_t element_size, mph_bool is_ptr) {
    MorphArray* arr = (MorphArray*)mph_alloc(ctx, sizeof(MorphArray), &mph_ti_array);
    mph_gc_push_root(ctx, (void**)&arr);
    arr->length = capacity;
    arr->capacity = capacity;
    arr->element_size = element_size;
    arr->elements_are_pointers = is_ptr;
    arr->data = mph_alloc(ctx, capacity * element_size, &mph_ti_raw);
    mph_gc_push_root(ctx, (void**)&arr->data);
    mph_gc_pop_roots(ctx, 2);
    return arr;
}

void* mph_array_at(MorphContext* ctx, MorphArray* arr, mph_int index) {
    mph_swap_in(ctx, arr);
    if (index < 0 || index >= arr->length) {
        printf("Panic: Array index out of bounds\n");
        exit(1);
    }
    // Deep Swap
    mph_swap_in(ctx, arr->data);
    return (uint8_t*)arr->data + (index * arr->element_size);
}

MorphArray* mph_array_concat(MorphContext* ctx, MorphArray* a, MorphArray* b) {
    mph_swap_in(ctx, a);
    mph_swap_in(ctx, b);
    // Deep Swap
    mph_swap_in(ctx, a->data);
    mph_swap_in(ctx, b->data);

    size_t new_len = a->length + b->length;
    MorphArray* res = mph_array_new(ctx, new_len, a->element_size, a->elements_are_pointers);

    // Copy a
    memcpy(res->data, a->data, a->length * a->element_size);
    // Copy b
    memcpy((uint8_t*)res->data + (a->length * a->element_size), b->data, b->length * b->element_size);

    return res;
}

// --- Maps ---

MorphMap* mph_map_new(MorphContext* ctx, MorphKeyKind kind, mph_bool val_is_ptr) {
    MorphMap* map = (MorphMap*)mph_alloc(ctx, sizeof(MorphMap), &mph_ti_map);
    mph_gc_push_root(ctx, (void**)&map);
    map->capacity = 16;
    map->count = 0;
    map->deleted_count = 0;
    map->key_kind = kind;
    map->values_are_pointers = val_is_ptr;
    map->entries = (MorphMapEntry*)mph_alloc(ctx, sizeof(MorphMapEntry) * map->capacity, &mph_ti_raw);
    mph_gc_push_root(ctx, (void**)&map->entries);
    memset(map->entries, 0, sizeof(MorphMapEntry) * map->capacity);
    mph_gc_pop_roots(ctx, 2);
    return map;
}

// Simplified Linear Probe Map Logic
uint64_t mph_hash_bytes(const void* data, size_t len) {
    uint64_t hash = 14695981039346656037ULL;
    const uint8_t* p = (const uint8_t*)data;
    for (size_t i = 0; i < len; i++) { hash ^= p[i]; hash *= 1099511628211ULL; }
    return hash;
}
// We update hash_key to take ctx
uint64_t mph_hash_key(MorphContext* ctx, void* key, MorphKeyKind kind) {
    if (kind == MPH_KEY_INT) return (uint64_t)key;
    if (kind == MPH_KEY_STRING) {
        MorphString* s = (MorphString*)key;
        mph_swap_in(ctx, s);
        return mph_hash_bytes(s->data, s->length);
    }
    return (uint64_t)key;
}

mph_bool mph_key_eq(MorphContext* ctx, void* k1, void* k2, MorphKeyKind kind) {
    if (kind == MPH_KEY_INT) return k1 == k2;
    if (kind == MPH_KEY_STRING) return mph_string_eq(ctx, (MorphString*)k1, (MorphString*)k2);
    return k1 == k2;
}

void mph_map_resize(MorphContext* ctx, MorphMap* map, size_t new_capacity) {
    MorphMapEntry* old_entries = map->entries;
    size_t old_capacity = map->capacity;

    map->entries = (MorphMapEntry*)mph_alloc(ctx, sizeof(MorphMapEntry) * new_capacity, &mph_ti_raw);
    memset(map->entries, 0, sizeof(MorphMapEntry) * new_capacity);
    map->capacity = new_capacity;
    map->count = 0;
    map->deleted_count = 0;

    mph_swap_in(ctx, old_entries);
    for (size_t i = 0; i < old_capacity; i++) {
        MorphMapEntry* e = &old_entries[i];
        if (e->occupied && !e->deleted) {
            uint64_t hash = mph_hash_key(ctx, e->key, map->key_kind);
            size_t idx = hash % map->capacity;
            while (1) {
                MorphMapEntry* slot = &map->entries[idx];
                if (!slot->occupied) {
                    slot->key = e->key;
                    slot->value = e->value;
                    slot->occupied = 1;
                    map->count++;
                    break;
                }
                idx = (idx + 1) % map->capacity;
            }
        }
    }
}

void mph_map_set(MorphContext* ctx, MorphMap* map, void* key, void* value) {
    mph_swap_in(ctx, map);
    mph_swap_in(ctx, map->entries);
    if (map->deleted_count >= map->capacity / 4) {
        mph_map_resize(ctx, map, map->capacity);
    }
    if (map->count >= map->capacity * 0.75) {
        size_t new_capacity = map->capacity * 2;
        if (new_capacity < 16) {
            new_capacity = 16;
        }
        mph_map_resize(ctx, map, new_capacity);
    }
    uint64_t hash = mph_hash_key(ctx, key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    size_t first_deleted = map->capacity;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) {
            if (first_deleted < map->capacity) {
                e = &map->entries[first_deleted];
                map->deleted_count--;
            }
            e->key = key; e->value = value; e->occupied = 1; e->deleted = 0; map->count++; return;
        }
        if (!e->deleted && mph_key_eq(ctx, e->key, key, map->key_kind)) {
            e->value = value; return;
        }
        if (e->deleted && first_deleted == map->capacity) {
            first_deleted = idx;
        }
        idx = (idx + 1) % map->capacity;
        if (idx == start) {
            if (first_deleted < map->capacity) {
                MorphMapEntry* slot = &map->entries[first_deleted];
                slot->key = key;
                slot->value = value;
                slot->occupied = 1;
                slot->deleted = 0;
                map->count++;
                map->deleted_count--;
            }
            return;
        }
    }
}
void* mph_map_get(MorphContext* ctx, MorphMap* map, void* key) {
    mph_swap_in(ctx, map);
    mph_swap_in(ctx, map->entries);
    uint64_t hash = mph_hash_key(ctx, key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) return NULL;
        if (!e->deleted && mph_key_eq(ctx, e->key, key, map->key_kind)) return e->value;
        idx = (idx + 1) % map->capacity;
        if (idx == start) return NULL;
    }
}
void mph_map_delete(MorphContext* ctx, MorphMap* map, void* key) {
     mph_swap_in(ctx, map);
     mph_swap_in(ctx, map->entries);
     uint64_t hash = mph_hash_key(ctx, key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) return;
        if (!e->deleted && mph_key_eq(ctx, e->key, key, map->key_kind)) {
            e->deleted = 1;
            map->count--;
            map->deleted_count++;
            if (map->deleted_count >= map->capacity / 4) {
                mph_map_resize(ctx, map, map->capacity);
            }
            return;
        }
        idx = (idx + 1) % map->capacity;
        if (idx == start) return;
    }
}
mph_int mph_map_len(MorphContext* ctx, MorphMap* map) {
    mph_swap_in(ctx, map);
    return map ? map->count : 0;
}

// --- Closures ---

MorphClosure* mph_closure_new(MorphContext* ctx, void* fn, void* env, int env_size) {
    MorphClosure* c = (MorphClosure*)mph_alloc(ctx, sizeof(MorphClosure), &mph_ti_closure);
    c->function = fn;
    c->env = env; // Assume env is already heap-allocated with TypeInfo
    return c;
}

// --- Channels ---
MorphChannel* mph_channel_new(MorphContext* ctx) {
    MorphChannel* c = (MorphChannel*)malloc(sizeof(MorphChannel));
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->cond_send, NULL);
    pthread_cond_init(&c->cond_recv, NULL);
    c->capacity = 5;
    c->buffer = (int64_t*)malloc(sizeof(int64_t) * c->capacity);
    c->count = 0; c->head = 0; c->tail = 0;
    return c;
}
void mph_channel_destroy(MorphContext* ctx, MorphChannel* c) {
    pthread_mutex_destroy(&c->lock);
    pthread_cond_destroy(&c->cond_send);
    pthread_cond_destroy(&c->cond_recv);
    free(c->buffer); free(c);
}
void mph_channel_send(MorphContext* ctx, MorphChannel* c, mph_int val) {
    pthread_mutex_lock(&c->lock);
    while (c->count == c->capacity) pthread_cond_wait(&c->cond_send, &c->lock);
    c->buffer[c->tail] = val;
    c->tail = (c->tail + 1) % c->capacity;
    c->count++;
    pthread_cond_signal(&c->cond_recv);
    pthread_mutex_unlock(&c->lock);
}
mph_int mph_channel_recv(MorphContext* ctx, MorphChannel* c) {
    pthread_mutex_lock(&c->lock);
    while (c->count == 0) pthread_cond_wait(&c->cond_recv, &c->lock);
    mph_int val = c->buffer[c->head];
    c->head = (c->head + 1) % c->capacity;
    c->count--;
    pthread_cond_signal(&c->cond_send);
    pthread_mutex_unlock(&c->lock);
    return val;
}
typedef struct ThreadArgs { MorphEntryFunction fn; void* arg; MorphContext ctx; } ThreadArgs;
void* mph_thread_wrapper(void* ptr) {
    ThreadArgs* args = (ThreadArgs*)ptr;
    mph_init_memory(&args->ctx);
    args->fn(&args->ctx, args->arg);
    mph_destroy_memory(&args->ctx);
    free(args);
    return NULL;
}
void mph_thread_spawn(MorphEntryFunction fn, void* arg) {
    pthread_t thread;
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
    args->fn = fn; args->arg = arg;
    pthread_create(&thread, NULL, mph_thread_wrapper, args);
    pthread_detach(thread);
}

// --- Interfaces ---
void* mph_assert_type(MorphContext* ctx, MorphInterface iface, mph_int expected_id) {
    if (iface.type_id == expected_id) {
         mph_swap_in(ctx, iface.instance);
         return iface.instance;
    }
    return NULL;
}

// --- Time ---
mph_int mph_time_Now(MorphContext* ctx, void* _env) {
    return (mph_int)mph_time_ms();
}
void mph_time_Sleep(MorphContext* ctx, void* _env, mph_int ms) {
    usleep(ms * 1000);
}

// --- IO (Minimal for Phase 1) ---
#define MAX_FILES 1024
FILE* mph_file_table[MAX_FILES];
int mph_file_count = 3;
typedef struct InternalFile { mph_int fd; } InternalFile;
void mph_init_files() { mph_file_table[0] = stdin; mph_file_table[1] = stdout; mph_file_table[2] = stderr; }
void mph_native_print_int(MorphContext* ctx, mph_int n) { printf("%ld\n", n); }
void mph_native_print(MorphContext* ctx, MorphString* s) {
    mph_swap_in(ctx, s);
    mph_swap_in(ctx, s->data);
    printf("%s\n", s->data);
}

// Helper to check if MorphInterface holds an Error
void mph_native_print_error(MorphContext* ctx, MorphError* err) {
    if (err) {
        mph_swap_in(ctx, err->message);
        printf("Error: %s\n", err->message->data);
    } else {
        printf("Error: (nil)\n");
    }
}

void* mph_io_make_file(MorphContext* ctx, void* _env, mph_int fd) {
    mph_init_files();
    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile), NULL);
    f->fd = fd;
    return f;
}

void* mph_io_Open(MorphContext* ctx, void* _env, MorphString* path) {
    mph_swap_in(ctx, path);
    mph_init_files();
    FILE* h = fopen(path->data, "r"); if(!h) return NULL;
    int fd = mph_file_count++; mph_file_table[fd] = h;
    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile), NULL); f->fd = fd; return f;
}
void* mph_io_Create(MorphContext* ctx, void* _env, MorphString* path) {
    mph_swap_in(ctx, path);
    mph_init_files();
    FILE* h = fopen(path->data, "w+"); if(!h) return NULL;
    int fd = mph_file_count++; mph_file_table[fd] = h;
    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile), NULL); f->fd = fd; return f;
}
MorphString* mph_io_Read(MorphContext* ctx, void* _env, void* f, mph_int size) {
    mph_init_files();
    if (!f) return mph_string_new(ctx, "");
    int fd = ((InternalFile*)f)->fd;
    char* buf = (char*)mph_alloc(ctx, size+1, &mph_ti_raw);
    size_t r = fread(buf, 1, size, mph_file_table[fd]);
    buf[r] = 0;
    MorphString* s = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    s->data = buf; s->length = r; return s;
}
mph_int mph_io_Write(MorphContext* ctx, void* _env, void* f, MorphString* s) {
    mph_swap_in(ctx, s);
    mph_init_files();
    if (!f) return -1;
    int fd = ((InternalFile*)f)->fd;
    return fwrite(s->data, 1, s->length, mph_file_table[fd]);
}
mph_int mph_io_Close(MorphContext* ctx, void* _env, void* f) {
    mph_init_files();
    if (!f) return -1;
    int fd = ((InternalFile*)f)->fd;
    if (fd <= 2) return 0;
    return fclose(mph_file_table[fd]);
}

// --- Native Implementations for Stdlib ---
// We match the name used by the compiler.
// (int, error) -> MorphTuple_Int_Error (simplified)
// But the compiler generates MorphTuple_<Hash>.
// Wait, the Compiler uses `getTupleCType`.
// For `(int, error)`, the name depends on `c.tupleTypes` state order?
// NO. If I hardcode the struct in Runtime, it will clash or mismatch.
//
// STRATEGY:
// 1. The Compiler generates `MorphTuple_Int_Error`.
// 2. The Runtime defines `MorphTuple_Int_Error` struct.
// 3. `mph_conv_Atoi` returns `MorphTuple_Int_Error`.

typedef struct MorphTuple_Int_Error {
    mph_int v0;
    MorphError* v1; // Error
} MorphTuple_Int_Error;

MorphError* mph_error_new(MorphContext* ctx, MorphString* msg) {
    int roots = 0;
    if (msg) { mph_gc_push_root(ctx, (void**)&msg); roots++; }
    MorphError* e = (MorphError*)mph_alloc(ctx, sizeof(MorphError), NULL); // Basic error
    mph_gc_push_root(ctx, (void**)&e);
    e->message = msg;
    mph_gc_pop_roots(ctx, roots + 1);
    return e;
}

MorphString* mph_conv_Itoa(MorphContext* ctx, void* _env, mph_int i) {
    char buf[32];
    sprintf(buf, "%ld", i);
    return mph_string_new(ctx, buf);
}

MorphTuple_Int_Error mph_conv_Atoi(MorphContext* ctx, void* _env, MorphString* s) {
    // Basic implementation
    mph_swap_in(ctx, s);
    char* end;
    long val = strtol(s->data, &end, 10);

    MorphTuple_Int_Error ret;
    if (end == s->data) {
        ret.v0 = 0;
        ret.v1 = mph_error_new(ctx, mph_string_new(ctx, "Invalid integer"));
    } else {
        ret.v0 = (mph_int)val;
        ret.v1 = NULL;
    }
    return ret;
}

// Entry point wrapper (if not provided by generated code)
int main() {
    MorphContext ctx;
    mph_init_memory(&ctx);
    morph_entry_point(&ctx);
    mph_destroy_memory(&ctx);
    return 0;
}
