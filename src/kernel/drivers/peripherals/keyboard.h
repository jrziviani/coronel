#ifndef KEYBOARD_H
#define KEYBOARD_H

class iarch;

#include "libs/functional.h"

namespace peripherals
{
    class keyboard
    {
    private:
        iarch *arch_;
        bool shift_ = false;

    public:
        keyboard(iarch *arch);

        void on_keyboard(const interrupt_t &interrupt);
        void on_keyup(unsigned char c);
        void on_keydown(unsigned char c);
    };

    keyboard &add_keyboard(iarch *arch);
}

using keyboard_handler_t = lib::interrupt_handler_type<peripherals::keyboard>;

#endif // KEYBOARD_H