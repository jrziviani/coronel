#ifndef ICXXABI_HPP
#define ICXXABI_HPP

// CODE FROM: https://wiki.osdev.org/C%2B%2B

// This header file provides declarations for functions and structures 
// used to manage the destruction of objects with static storage duration 
// in C++ programs. It is part of the C++ ABI (Application Binary Interface) 
// and is commonly used in environments where custom runtime support is required.

// The maximum number of functions that can be registered with `__cxa_atexit`.
const unsigned int ATEXIT_MAX_FUNCS = 128;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// A platform-independent type for unsigned architecture-specific values.
typedef unsigned uarch_t;

// This structure represents an entry in the atexit function table. 
// It is used to store information about destructors for objects 
// with static storage duration.
struct atexit_func_entry_t
{
    /*
     * Each member is at least 4 bytes large. Such that each entry is 12bytes.
     * 128 * 12 = 1.5KB exact.
     **/

    // Pointer to the destructor function for the object.
    void (*destructor_func)(void *);

    // Pointer to the object to be destroyed.
    void *obj_ptr;

    // Handle for the shared object (DSO) that owns the object.
    // This is used to ensure proper cleanup in dynamic linking scenarios.
    void *dso_handle;
};

// Registers a destructor function to be called when the program exits.
// Parameters:
// - f: Pointer to the destructor function.
// - objptr: Pointer to the object to be destroyed.
// - dso: Handle for the shared object (DSO) that owns the object.
// Returns 0 on success, or a non-zero value on failure.
int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);

// Calls all registered destructors for objects with static storage duration.
// If `f` is non-null, only destructors associated with the given DSO handle 
// are called. If `f` is null, all destructors are called.
void __cxa_finalize(void *f);


#ifdef __cplusplus
};
#endif // __cplusplus

#endif // ICXXABI_HPP