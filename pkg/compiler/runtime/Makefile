# Makefile for Morph Memory V2 Testing
# Week 1: Build and test foundation

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread -O2 -g
LDFLAGS = -pthread

# Targets
.PHONY: all test clean

all: test

test: morph_mem_v2_test
	@echo "Running Week 1 tests..."
	./morph_mem_v2_test

morph_mem_v2_test: morph_mem_v2_test.c morph_mem_v2.c morph_mem_v2.h
	$(CC) $(CFLAGS) -o morph_mem_v2_test morph_mem_v2_test.c morph_mem_v2.c $(LDFLAGS)

clean:
	rm -f morph_mem_v2_test *.o /tmp/morph_mem_stats.json

.DEFAULT_GOAL := test
