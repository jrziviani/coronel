#ifndef STDLIB_HPP
#define STDLIB_HPP

#include "stdint.hpp"

enum base {
    bin = 2,
    oct = 8,
    dec = 10,
    hex = 16
};

static void itoa(char *buffer, size_t size, int64_t value, base b)
{
    bool negative = false;
    uint64_t uvalue = 0;

    if (size <= 1) {
        return;
    }

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (b == dec && value < 0) {
        negative = true;
        value = -value;
    }
    uvalue = static_cast<uint64_t>(value);

    size_t i = 0;
    while (uvalue)
    {
        uint8_t digit = uvalue % b;
        buffer[i++] = (digit < 10) ? '0' + digit : 'a' + digit - 10;
        uvalue /= b;
    }

    if (negative) {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';

    i--;
    for (size_t j = 0; j < i; j++, i--) {
        char temp = buffer[j];
        buffer[j] = buffer[i];
        buffer[i] = temp;
    }  
}

#endif // STDLIB_HPP