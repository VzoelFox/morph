#!/bin/bash
# N1 Test Runner - Automated Testing
# Auto-discover dan run all test files dengan timeout protection

# Note: We handle errors manually, no set -e

# Colors untuk output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
START_TIME=$(date +%s)

# Arrays to track failures
declare -a FAILED_FILES
declare -a FAILED_REASONS

echo "╔════════════════════════════════════════╗"
echo "║  N1 Test Runner - Automated Testing   ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Mode selection: regression (default) or all
MODE="${1:-regression}"

if [ "$MODE" == "regression" ]; then
    # Read curated regression suite
    if [ ! -f "n1/regression_suite.txt" ]; then
        echo "${RED}❌ Regression suite file not found: n1/regression_suite.txt${NC}"
        exit 1
    fi

    # Read files from regression suite (skip comments and empty lines)
    TEST_FILES=$(grep -v '^#' n1/regression_suite.txt | grep -v '^$' | sort)

    echo "${BLUE}Mode: Regression Suite (curated tests)${NC}"
elif [ "$MODE" == "all" ]; then
    # Find all test files
    TEST_FILES=$(find n1 -name "test_*.fox" -type f | sort)
    echo "${BLUE}Mode: All Tests (auto-discover)${NC}"
else
    echo "${RED}❌ Invalid mode: $MODE${NC}"
    echo "Usage: $0 [regression|all]"
    exit 1
fi

if [ -z "$TEST_FILES" ]; then
    echo "${RED}❌ No test files found${NC}"
    exit 1
fi

# Count total tests
TOTAL_TESTS=$(echo "$TEST_FILES" | wc -l)

echo "Found $TOTAL_TESTS test files"
echo ""

# Test each file
TEST_NUM=0
for TEST_FILE in $TEST_FILES; do
    ((TEST_NUM++))

    # Extract base name (without .fox extension)
    BASE_NAME="${TEST_FILE%.fox}"
    TEST_NAME=$(basename "$BASE_NAME")

    echo -e "${BLUE}[$TEST_NUM/$TOTAL_TESTS]${NC} Testing $TEST_FILE"

    # Step 1: Compile test
    echo -n "  Compiling... "
    COMPILE_START=$(date +%s)

    if timeout 60s ./morph build "$TEST_FILE" > /tmp/test_compile_$$.log 2>&1; then
        COMPILE_END=$(date +%s)
        COMPILE_TIME=$((COMPILE_END - COMPILE_START))
        echo -e "${GREEN}✅ SUCCESS${NC} (${COMPILE_TIME}s)"
    else
        COMPILE_EXIT_CODE=$?
        echo -e "${RED}❌ FAILED${NC}"

        if [ $COMPILE_EXIT_CODE -eq 124 ]; then
            FAILED_REASON="Compile timeout (>60s)"
        else
            FAILED_REASON="Compile error (see below)"
            echo ""
            echo "  ${YELLOW}Compilation errors:${NC}"
            tail -10 /tmp/test_compile_$$.log | sed 's/^/    /'
        fi

        FAILED_FILES+=("$TEST_FILE")
        FAILED_REASONS+=("$FAILED_REASON")
        ((FAILED_TESTS++))
        echo ""
        continue
    fi

    # Step 2: Run test binary
    echo -n "  Running... "
    RUN_START=$(date +%s)

    if timeout 30s "$BASE_NAME" > /tmp/test_run_$$.log 2>&1; then
        RUN_END=$(date +%s)
        RUN_TIME=$((RUN_END - RUN_START))
        echo -e "${GREEN}✅ SUCCESS${NC} (${RUN_TIME}s)"
        ((PASSED_TESTS++))
    else
        RUN_EXIT_CODE=$?
        echo -e "${RED}❌ FAILED${NC}"

        if [ $RUN_EXIT_CODE -eq 124 ]; then
            FAILED_REASON="Runtime timeout (>30s)"
        else
            FAILED_REASON="Runtime error (exit code: $RUN_EXIT_CODE)"
            echo ""
            echo "  ${YELLOW}Runtime output:${NC}"
            tail -20 /tmp/test_run_$$.log | sed 's/^/    /'
        fi

        FAILED_FILES+=("$TEST_FILE")
        FAILED_REASONS+=("$FAILED_REASON")
        ((FAILED_TESTS++))
    fi

    echo ""
done

# Cleanup temp files
rm -f /tmp/test_compile_$$.log /tmp/test_run_$$.log

# Calculate total time
END_TIME=$(date +%s)
TOTAL_TIME=$((END_TIME - START_TIME))

# Print summary
echo "════════════════════════════════════════"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}✅ Summary: $PASSED_TESTS/$TOTAL_TESTS PASSED (100%)${NC}"
    echo "Total time: ${TOTAL_TIME}s"
    echo "════════════════════════════════════════"
    exit 0
else
    echo -e "${RED}❌ Summary: $PASSED_TESTS/$TOTAL_TESTS PASSED ($(( PASSED_TESTS * 100 / TOTAL_TESTS ))%)${NC}"
    echo "Total time: ${TOTAL_TIME}s"
    echo ""
    echo -e "${YELLOW}Failed tests:${NC}"
    for i in "${!FAILED_FILES[@]}"; do
        echo "  $((i+1)). ${FAILED_FILES[$i]}"
        echo "     Reason: ${FAILED_REASONS[$i]}"
    done
    echo "════════════════════════════════════════"
    exit 1
fi
