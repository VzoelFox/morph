package runtime

import (
	"strings"
	"testing"
)

func TestRuntimeMapSwapInEntries(t *testing.T) {
	source := RuntimeSource

	swapCalls := strings.Count(source, "mph_swap_in(ctx, map->entries);")
	if swapCalls < 3 {
		t.Fatalf("expected swap-in for map->entries in set/get/delete, found %d", swapCalls)
	}

	if !strings.Contains(source, "void* mph_map_get(MorphContext* ctx, MorphMap* map, void* key)") {
		t.Fatalf("expected mph_map_get definition")
	}
	if !strings.Contains(source, "void mph_map_set(MorphContext* ctx, MorphMap* map, void* key, void* value)") {
		t.Fatalf("expected mph_map_set definition")
	}
	if !strings.Contains(source, "void mph_map_delete(MorphContext* ctx, MorphMap* map, void* key)") {
		t.Fatalf("expected mph_map_delete definition")
	}
}

func TestRuntimeMapResize(t *testing.T) {
	source := RuntimeSource

	if !strings.Contains(source, "void mph_map_resize(MorphContext* ctx, MorphMap* map, size_t new_capacity)") {
		t.Fatalf("expected mph_map_resize definition")
	}
	if !strings.Contains(source, "mph_map_resize(ctx, map, new_capacity)") {
		t.Fatalf("expected mph_map_resize invocation in mph_map_set")
	}
}

func TestRuntimeMapTombstoneCleanup(t *testing.T) {
	source := RuntimeSource

	if !strings.Contains(source, "deleted_count") {
		t.Fatalf("expected deleted_count tracking in runtime source")
	}
	if !strings.Contains(source, "mph_map_resize(ctx, map, map->capacity)") {
		t.Fatalf("expected tombstone cleanup to rehash map at current capacity")
	}
}

func TestRuntimeMapHeaderFields(t *testing.T) {
	header := MorphHeader

	if !strings.Contains(header, "size_t deleted_count;") {
		t.Fatalf("expected deleted_count field in MorphMap header")
	}
}

func TestRuntimePageFreeListFields(t *testing.T) {
	header := MorphHeader

	if !strings.Contains(header, "ObjectHeader* free_list;") {
		t.Fatalf("expected per-page free_list field")
	}
	if !strings.Contains(header, "struct ObjectHeader* free_next;") {
		t.Fatalf("expected free_next field on ObjectHeader")
	}
	if !strings.Contains(header, "struct ObjectHeader* page_free_next;") {
		t.Fatalf("expected page_free_next field on ObjectHeader")
	}
}

func TestRuntimeMarkStackBlockLayout(t *testing.T) {
	header := MorphHeader
	source := RuntimeSource

	if !strings.Contains(header, "MARK_STACK_BLOCK_SIZE") {
		t.Fatalf("expected MARK_STACK_BLOCK_SIZE definition")
	}
	if !strings.Contains(header, "typedef struct MarkStackBlock") {
		t.Fatalf("expected MarkStackBlock definition")
	}
	if !strings.Contains(header, "MarkStackBlock* head;") {
		t.Fatalf("expected MarkStack head pointer")
	}
	if !strings.Contains(source, "mph_mark_stack_pop") {
		t.Fatalf("expected mark stack pop helper")
	}
}
