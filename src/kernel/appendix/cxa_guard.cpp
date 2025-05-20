// This file implements the C++ ABI (Application Binary Interface) guard functions 
// used for thread-safe initialization of static local variables. These functions 
// are part of the Itanium C++ ABI and are called by the compiler to ensure that 
// static local variables are initialized only once, even in multithreaded environments.

#include "libs/stdint.hpp"

namespace __cxxabiv1
{
    using guard_type = uint64_t;

    extern "C"
    {
        // This function checks if the static local variable has already been initialized.
        // If the guard variable is zero, it means the variable has not been initialized yet.
        // Returns 1 if the variable needs initialization, 0 otherwise.
        int __cxa_guard_acquire(guard_type *g)
        {
            return !(*g);
        }

        // This function marks the static local variable as initialized by setting the 
        // guard variable to 1. This ensures that subsequent calls to __cxa_guard_acquire 
        // will return 0.
        void __cxa_guard_release(guard_type *g)
        {
            *g = 1;
        }

        // This function is called if the initialization of the static local variable 
        // is aborted (e.g., due to an exception). It allows for cleanup or rollback 
        // if necessary. In this implementation, it does nothing.
        void __cxa_guard_abort(guard_type *g)
        {
            (void)g;
        }
    }
}