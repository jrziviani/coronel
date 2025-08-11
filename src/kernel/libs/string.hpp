#ifndef STRING_HPP
#define STRING_HPP

#include "stdint.hpp"

namespace lib
{
    inline size_t strlen(const char *str)
    {
        size_t len = 0;
        while (str[len] != '\0') {
            len++;
        }
        return len;
    }

    inline bool isdigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    inline void *memset(void *ptr, unsigned char ch, size_t count)
    {
        unsigned char *p = static_cast<unsigned char *>(ptr);
        while (count > 0) {
            *p++ = ch;
            count--;
        }
        
        return ptr;
    }

    inline void *memcpy(void *dest, const void *src, size_t count)
    {
        unsigned char *d = static_cast<unsigned char *>(dest);
        const unsigned char *s = static_cast<const unsigned char *>(src);
        
        while (count > 0) {
            *d++ = *s++;
            count--;
        }
        
        return dest;
    }

    inline void *memmove(void *dest, const void *src, size_t count)
    {
        unsigned char *d = static_cast<unsigned char *>(dest);
        const unsigned char *s = static_cast<const unsigned char *>(src);
        
        if (d < s) {
            while (count > 0) {
                *d++ = *s++;
                count--;
            }
        } else {
            d += count;
            s += count;
            while (count > 0) {
                *(--d) = *(--s);
                count--;
            }
        }
        
        return dest;
    }
}

#endif // STRING_HPP