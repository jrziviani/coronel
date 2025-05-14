#ifndef IARCH_H
#define IARCH_H

#include <libs/stdint.h>

constexpr size_t WIDTH = 80;
constexpr size_t HEIGHT = 25;

class iprotected_mode
{
protected:
    enum colors
    {
        BLACK = 0, BLUE, GREEN, CYAN, RED, PURPLE, BROWN,
        GRAY, DARK_GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN,
        LIGHT_RED, LIGHT_PURPLE, YELLOW, WHITE
    } current_color_;

    uint8_t col_ = 0;
    uint8_t lin_ = 0;

public:
    iprotected_mode(colors color = colors::GRAY)
        : current_color_(color), col_(0), lin_(0) {}

    virtual void clear() = 0;
    virtual void printc(char c) = 0;
    virtual void prints(const char *s) = 0;
    virtual void printd(int d) = 0;
    virtual void printx(uint64_t x) = 0;

    void set_color(colors color = colors::GRAY)
    {
        current_color_ = color;
    }

    void column(uint8_t col)
    {
        col_ = (col > WIDTH) ? 0 : col;
    }

    uint8_t ncolumns() const
    {
        return col_;
    }

    void line(uint8_t lin)
    {
        lin_ = (lin > HEIGHT) ? 0 : lin;
    }

    uint8_t nlines() const
    {
        return lin_;
    }
};

class iarch
{
public:
    virtual void cpu_halt() = 0;
    virtual iprotected_mode *get_video() = 0;

    virtual uint8_t read_byte(uint32_t addr, uint16_t addr_port, uint16_t value_port) const = 0;
    virtual uint16_t read_word(uint32_t addr, uint16_t addr_port, uint16_t value_port) const = 0;
    virtual uint32_t read_dword(uint32_t addr, uint16_t addr_port, uint16_t value_port) const = 0;
    virtual void write_byte(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint8_t value) const = 0;
    virtual void write_word(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint16_t value) const = 0;
    virtual void write_dword(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint32_t value) const = 0;
};

#endif // IARCH_H