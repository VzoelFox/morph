# Makefile for Morph Memory V2 Testing
# Week 2: Arena Allocator + Foundation

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread -O2 -g
LDFLAGS = -pthread

# Targets
.PHONY: all test test-week1 test-week2 benchmark clean

all: test

test: test-week1 test-week2

test-week1: morph_mem_v2_test
	@echo "=== Running Week 1 Foundation Tests ==="
	./morph_mem_v2_test

test-week2: morph_mem_v2_arena_test
	@echo ""
	@echo "=== Running Week 2 Arena Tests ==="
	./morph_mem_v2_arena_test

benchmark: morph_mem_v2_arena_test
	@echo "=== Running Arena Benchmarks ==="
	./morph_mem_v2_arena_test

morph_mem_v2_test: morph_mem_v2_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_test morph_mem_v2_test.c morph_mem_v2.c $(LDFLAGS)

morph_mem_v2_arena_test: morph_mem_v2_arena_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_arena_test morph_mem_v2_arena_test.c morph_mem_v2.c $(LDFLAGS)

clean:
	rm -f morph_mem_v2_test morph_mem_v2_arena_test *.o /tmp/morph_mem_stats.json

.DEFAULT_GOAL := test
