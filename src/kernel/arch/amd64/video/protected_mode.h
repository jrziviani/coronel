#ifndef PROTECTED_MODE_H
#define PROTECTED_MODE_H

#include <libs/stdint.h>
#include <arch/iarch.h>

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
};

#endif // PROTECTED_MODE_H