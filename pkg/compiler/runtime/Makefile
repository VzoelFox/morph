# Makefile for Morph Memory V2 Testing
# Week 5-6: Integration + Pool + Arena + Foundation
# Week 12: GC Optimization Tests

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread -O2 -g -DUSE_MEMORY_V2=1
LDFLAGS = -pthread

# Targets
.PHONY: all test test-week1 test-week2 test-week3 test-bridge test-gc test-optimization benchmark clean

all: test

test: test-week1 test-week2 test-week3 test-bridge test-gc test-optimization

test-week1: morph_mem_v2_test
	@echo "=== Running Week 1 Foundation Tests ==="
	./morph_mem_v2_test

test-week2: morph_mem_v2_arena_test
	@echo ""
	@echo "=== Running Week 2 Arena Tests ==="
	./morph_mem_v2_arena_test

test-week3: morph_mem_v2_pool_test
	@echo ""
	@echo "=== Running Week 3-4 Pool Tests ==="
	./morph_mem_v2_pool_test

test-bridge: morph_mem_v2_bridge_test
	@echo ""
	@echo "=== Running Week 5-6 Bridge Integration Tests ==="
	./morph_mem_v2_bridge_test

test-gc: morph_mem_v2_gc_test
	@echo ""
	@echo "=== Running Week 7-8 Generational GC Tests ==="
	./morph_mem_v2_gc_test

test-optimization: morph_mem_v2_optimization_test
	@echo ""
	@echo "=== Running Week 12 GC Optimization Tests ==="
	./morph_mem_v2_optimization_test

benchmark: morph_mem_v2_arena_test morph_mem_v2_pool_test morph_mem_v2_bridge_test morph_mem_v2_gc_test morph_mem_v2_optimization_test
	@echo "=== Running All Benchmarks ==="
	@echo ""
	@echo "--- Arena Benchmarks ---"
	./morph_mem_v2_arena_test | grep -A 20 "Arena vs Malloc"
	@echo ""
	@echo "--- Pool Benchmarks ---"
	./morph_mem_v2_pool_test | grep -A 20 "Pool vs Malloc"

morph_mem_v2_test: morph_mem_v2_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_test morph_mem_v2_test.c morph_mem_v2.c $(LDFLAGS)

morph_mem_v2_arena_test: morph_mem_v2_arena_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_arena_test morph_mem_v2_arena_test.c morph_mem_v2.c $(LDFLAGS)

morph_mem_v2_pool_test: morph_mem_v2_pool_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_pool_test morph_mem_v2_pool_test.c morph_mem_v2.c $(LDFLAGS)

morph_mem_v2_bridge_test: morph_mem_v2_bridge_test.c morph_mem_v2_bridge.c morph_mem_v2.c morph_mem_v2.h morph_mem_v2_bridge.h
	$(CC) $(CFLAGS) -o morph_mem_v2_bridge_test morph_mem_v2_bridge_test.c morph_mem_v2_bridge.c morph_mem_v2.c $(LDFLAGS)

morph_mem_v2_gc_test: morph_mem_v2_gc_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_gc_test morph_mem_v2_gc_test.c morph_mem_v2.c $(LDFLAGS)

morph_mem_v2_optimization_test: morph_mem_v2_optimization_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_optimization_test morph_mem_v2_optimization_test.c morph_mem_v2.c $(LDFLAGS)

clean:
	rm -f morph_mem_v2_test morph_mem_v2_arena_test morph_mem_v2_pool_test morph_mem_v2_bridge_test morph_mem_v2_gc_test morph_mem_v2_optimization_test *.o /tmp/morph_mem_stats.json /tmp/morph_bridge_stats.json

.DEFAULT_GOAL := test
