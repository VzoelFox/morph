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
