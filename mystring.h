#ifndef __MYSTRING_H__
#define __MYSTRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

size_t strlen_utf16(const uint16_t *utf16be_str);
void ascii_to_utf16be(const char *ascii_str, uint16_t *utf16be_str, size_t utf16be_size);
void utf16be_to_ascii(const uint16_t *utf16be_str, char *ascii_str, size_t ascii_size);
void ascii_hex_str_to_utf16(const char *ascii_hex_str, uint16_t *utf16be_str, size_t utf16be_size);
void utf16be_to_ascii_hex_str(const uint16_t *utf16be_str, char *ascii_str, size_t ascii_size);

#ifdef __cplusplus
}
#endif

#endif /* __MYSTRING_H__ */