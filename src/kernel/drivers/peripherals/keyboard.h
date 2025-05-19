#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <arch/iarch.h>

class keyboard
{
private:
    iarch *arch_;
    bool shift_ = false;

public:
    keyboard(iarch *arch) : arch_(arch) {}

    void on_keyboard();
    void on_keyup(unsigned char c);
    void on_keydown(unsigned char c);
};

#endif // KEYBOARD_H