#ifndef PROTECTED_MODE_HPP
#define PROTECTED_MODE_HPP

#include "libs/stdint.hpp"
#include "arch/iarch.hpp"

class protected_mode : public iprotected_mode
{
    int16_t buffer_[WIDTH * (HEIGHT * 2)];

private:
    void scroll_down();

public:
    protected_mode();

    void clear() override;
    void printc(char c) override;
    void prints(const char *s) override;
    void printd(int d);
    void printx(uint64_t x);
};

#endif // PROTECTED_MODE_HPP