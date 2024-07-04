/*
 * Standard Library Functions for Working with ASCII Strings in C
 *
 * <string.h> functions:
 * 
 * 1. strcpy(dest, src)
 *    Copies the string pointed to by src (including the null terminator) to the array pointed to by dest.
 * 
 * 2. strncpy(dest, src, n)
 *    Copies up to n characters from the string pointed to by src to dest.
 * 
 * 3. strcat(dest, src)
 *    Appends the string pointed to by src to the end of the string pointed to by dest.
 * 
 * 4. strncat(dest, src, n)
 *    Appends up to n characters from the string pointed to by src to dest.
 * 
 * 5. strcmp(str1, str2)
 *    Compares the string pointed to by str1 to the string pointed to by str2.
 * 
 * 6. strncmp(str1, str2, n)
 *    Compares up to n characters of the string pointed to by str1 to the string pointed to by str2.
 * 
 * 7. strlen(str)
 *    Returns the length of the string pointed to by str, excluding the null terminator.
 * 
 * 8. strchr(str, c)
 *    Returns a pointer to the first occurrence of the character c in the string str.
 * 
 * 9. strrchr(str, c)
 *    Returns a pointer to the last occurrence of the character c in the string str.
 * 
 * 10. strstr(haystack, needle)
 *     Returns a pointer to the first occurrence of the substring needle in the string haystack.
 * 
 * 11. strtok(str, delim)
 *     Breaks the string str into a series of tokens separated by delim.
 * 
 * <ctype.h> functions:
 * 
 * 1. isalpha(c)
 *    Checks if the character c is an alphabetic letter.
 * 
 * 2. isdigit(c)
 *    Checks if the character c is a decimal digit.
 * 
 * 3. isalnum(c)
 *    Checks if the character c is an alphanumeric character.
 * 
 * 4. isspace(c)
 *    Checks if the character c is a whitespace character.
 * 
 * 5. isupper(c)
 *    Checks if the character c is an uppercase letter.
 * 
 * 6. islower(c)
 *    Checks if the character c is a lowercase letter.
 * 
 * 7. toupper(c)
 *    Converts the character c to uppercase, if possible.
 * 
 * 8. tolower(c)
 *    Converts the character c to lowercase, if possible.
 *
 * These functions cover a broad range of common string operations such as copying, concatenation,
 * comparison, length calculation, searching, tokenizing, and character classification.
 */

#include "mystring.h"
#include <stdio.h>
#include <string.h>

#define STRING_LENGTH_MAX 256 

size_t strlen_utf16(const uint16_t *utf16be_str)
{
    size_t length = 0;
    
    while (utf16be_str[length] != 0x0000) {
        length++;
    }
    return length;
}

// Function to convert ASCII to UTF-16 BE
void ascii_to_utf16be(const char *ascii_str, uint16_t *utf16be_str, size_t utf16be_size)
{
    while (*ascii_str != '\0' && utf16be_size > 0) {
        // UTF-16 BE representation: High byte first, low byte second
        // High byte (always 0 for ASCII), Low byte (ASCII character)
        *utf16be_str++ = (uint16_t)(*ascii_str++ & 0x00FF);
        utf16be_size--;
    }
    *utf16be_str++ = 0x0000; // Null-terminate the UTF-16 string
}

// Function to convert UTF-16 BE to ASCII
void utf16be_to_ascii(const uint16_t *utf16be_str, char *ascii_str, size_t ascii_size)
{
    while (*utf16be_str != 0x0000 && ascii_size > 0) {
        // Only convert if it's a valid ASCII character
        if (*utf16be_str <= 0x007F) {
            *ascii_str++ = (uint8_t)(*utf16be_str++ & 0x00FF);
        } else {
            // Handle non-ASCII characters if needed (for now, let's replace with '?')
            *ascii_str++ = '?';
        }
        ascii_size--;
    }
    *ascii_str = '\0'; // Null-terminate the ASCII string
}

int main(void)
{
    char str_ascii[] = "Hello 120, 5:30 end";
    char str_ascii_1[STRING_LENGTH_MAX];
    uint16_t str_utf16be[STRING_LENGTH_MAX];

    printf("ASCII to UTF-16BE array\nASCII string is: %s\n", str_ascii);

    // ASCII to UTF-16BE array
    ascii_to_utf16be(str_ascii, str_utf16be, STRING_LENGTH_MAX);
    size_t utf16_len = strlen_utf16(str_utf16be);

    printf("UTF-16 BE array is: ");
    for (int i = 0; i < utf16_len; ++i) {
        printf("%x", (uint16_t)str_utf16be[i]);
    }

    // UTF-16BE array to ASII string
    utf16be_to_ascii(str_utf16be, str_ascii_1, STRING_LENGTH_MAX);
    printf("\nReturn to ASCII string is: %s", str_ascii_1);

    char str_ascii_utf16[STRING_LENGTH_MAX];

    return 0;
}
