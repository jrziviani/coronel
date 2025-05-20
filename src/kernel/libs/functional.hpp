#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#include "type_traits.hpp"
#include "arch/amd64/registers.hpp"

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

#endif // FUNCTIONAL_HPP