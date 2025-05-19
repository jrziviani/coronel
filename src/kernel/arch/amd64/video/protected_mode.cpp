#include "protected_mode.h"
#include "config.h"

#include <stdarg.h>
#include <libs/stdlib.h>

protected_mode::protected_mode() : iprotected_mode()
{
}

void protected_mode::scroll_down()
{
    auto videobuf = reinterpret_cast<uint16_t*>(VGA_VIRTUAL_ADDRESS);
    for (size_t y = 1; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            videobuf[(y - 1) * WIDTH + x] = videobuf[y * WIDTH + x];
        }
    }

    for (size_t x = 0; x < WIDTH; x++) {
        videobuf[(HEIGHT - 1) * WIDTH + x] = ' ' | 1 << 8;
    }

    lin_ = HEIGHT - 1;
}

void protected_mode::clear()
{
    current_color_ = colors::GRAY;

    auto videobuf = reinterpret_cast<uint16_t*>(VGA_VIRTUAL_ADDRESS);
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            videobuf[y * WIDTH + x] = ' ' | 1 << 8;
        }
    }

    lin_ = 0;
    col_ = 0;
}

void protected_mode::printc(char c)
{
    auto videobuf = reinterpret_cast<uint16_t*>(VGA_VIRTUAL_ADDRESS);

    if (c == '\n' || c == '\r') {
        col_ = 0;
        lin_++;
        if (lin_ >= HEIGHT) {
            scroll_down();
        }
        return;
    }

    videobuf[lin_ * WIDTH + col_] = c | current_color_ << 8;
    col_++;
    if (col_ >= WIDTH) {
        col_ = 0;
        lin_++;
        if (lin_ >= HEIGHT) {
            scroll_down();
        }
    }
}

void protected_mode::prints(const char *s)
{
    while (*s) {
        printc(*s);
        s++;
    }
}

void protected_mode::printd(int d)
{
    char buffer[32];
    itoa(buffer, 32, d, base::dec);
    prints(buffer);
}

void protected_mode::printx(uint64_t x)
{
    char buffer[32];
    itoa(buffer, 32, x, base::hex);
    prints(buffer);
}