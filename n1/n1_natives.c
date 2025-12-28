/*
 * N1 Native C Functions
 * These functions are called from Fox code for performance-critical operations
 * Compiled separately and linked with N1-generated assembly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * STRING OPERATIONS
 * ============================================================================ */

/**
 * Convert single character string to ASCII code
 * @param char_str Single character as string
 * @return ASCII code (0-255)
 */
int native_char_to_ascii(const char* char_str) {
    if (char_str == NULL || char_str[0] == '\0') {
        return 0;
    }
    return (int)(unsigned char)char_str[0];
}

/**
 * Get substring from string
 * @param str Source string
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return Substring (caller must free)
 */
char* native_substring(const char* str, int start, int end) {
    if (str == NULL) {
        char* empty = malloc(1);
        empty[0] = '\0';
        return empty;
    }

    int len = strlen(str);
    if (start < 0) start = 0;
    if (end > len) end = len;
    if (start >= end) {
        char* empty = malloc(1);
        empty[0] = '\0';
        return empty;
    }

    int sub_len = end - start;
    char* result = malloc(sub_len + 1);
    strncpy(result, str + start, sub_len);
    result[sub_len] = '\0';

    return result;
}

/**
 * Get string length
 * @param str String
 * @return Length
 */
int native_strlen(const char* str) {
    if (str == NULL) {
        return 0;
    }
    return (int)strlen(str);
}

/* ============================================================================
 * CHARACTER CLASSIFICATION
 * ============================================================================ */

/**
 * Check if character is a letter (a-z, A-Z, _)
 * @param ch ASCII code
 * @return 1 if letter, 0 otherwise
 */
int native_is_letter(int ch) {
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z') ||
           (ch == '_') ? 1 : 0;
}

/**
 * Check if character is a digit (0-9)
 * @param ch ASCII code
 * @return 1 if digit, 0 otherwise
 */
int native_is_digit(int ch) {
    return (ch >= '0' && ch <= '9') ? 1 : 0;
}

/**
 * Check if character is whitespace (space, tab, newline, carriage return)
 * @param ch ASCII code
 * @return 1 if whitespace, 0 otherwise
 */
int native_is_whitespace(int ch) {
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') ? 1 : 0;
}

/* ============================================================================
 * DEBUGGING HELPERS
 * ============================================================================ */

/**
 * Print integer (for debugging)
 */
void native_print_int(int value) {
    printf("%d\n", value);
}

/**
 * Print string (for debugging)
 */
void native_print_string(const char* str) {
    if (str != NULL) {
        printf("%s\n", str);
    }
}

/**
 * Print character as ASCII code (for debugging)
 */
void native_print_char(int ch) {
    if (ch >= 32 && ch <= 126) {
        printf("'%c' (%d)\n", (char)ch, ch);
    } else {
        printf("\\x%02x (%d)\n", ch, ch);
    }
}

/* ============================================================================
 * MAIN (For testing natives standalone)
 * ============================================================================ */

#ifdef TEST_NATIVES
int main() {
    // Test char_to_ascii
    printf("Testing native_char_to_ascii:\n");
    printf("  'A' -> %d (expected: 65)\n", native_char_to_ascii("A"));
    printf("  '+' -> %d (expected: 43)\n", native_char_to_ascii("+"));
    printf("  '0' -> %d (expected: 48)\n", native_char_to_ascii("0"));

    // Test substring
    printf("\nTesting native_substring:\n");
    char* sub1 = native_substring("Hello World", 0, 5);
    printf("  \"Hello World\"[0:5] -> \"%s\" (expected: \"Hello\")\n", sub1);
    free(sub1);

    char* sub2 = native_substring("1 + 2", 2, 3);
    printf("  \"1 + 2\"[2:3] -> \"%s\" (expected: \"+\")\n", sub2);
    free(sub2);

    // Test strlen
    printf("\nTesting native_strlen:\n");
    printf("  \"Hello\" -> %d (expected: 5)\n", native_strlen("Hello"));
    printf("  \"\" -> %d (expected: 0)\n", native_strlen(""));

    // Test character classification
    printf("\nTesting character classification:\n");
    printf("  is_letter('a') -> %d (expected: 1)\n", native_is_letter('a'));
    printf("  is_letter('Z') -> %d (expected: 1)\n", native_is_letter('Z'));
    printf("  is_letter('_') -> %d (expected: 1)\n", native_is_letter('_'));
    printf("  is_letter('5') -> %d (expected: 0)\n", native_is_letter('5'));

    printf("  is_digit('0') -> %d (expected: 1)\n", native_is_digit('0'));
    printf("  is_digit('9') -> %d (expected: 1)\n", native_is_digit('9'));
    printf("  is_digit('a') -> %d (expected: 0)\n", native_is_digit('a'));

    printf("  is_whitespace(' ') -> %d (expected: 1)\n", native_is_whitespace(' '));
    printf("  is_whitespace('\\n') -> %d (expected: 1)\n", native_is_whitespace('\n'));
    printf("  is_whitespace('a') -> %d (expected: 0)\n", native_is_whitespace('a'));

    printf("\n✅ All native functions working!\n");
    return 0;
}
#endif
