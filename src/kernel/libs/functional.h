#ifndef FUNCTIONAL_H
#define FUNCTIONAL_H

#include "type_traits.h"
#include "arch/amd64/registers.h"

namespace lib
{
    template <typename T>
    struct interrupt_handler_type
    {
        using THandler = void (T::*)(const interrupt_t &);
        
        T &instance;
        THandler method;

        interrupt_handler_type(T &object, THandler hwnd) : instance(object), method(hwnd) {}

        void operator()(const interrupt_t &interrupt) const
        {
            (instance.*method)(interrupt);
        }
    };
}

#endif // FUNCTIONAL_H