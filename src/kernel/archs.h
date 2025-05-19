#include <arch/amd64/amd64.h>
#include <memory/allocators.h>
#include <libs/new.h>

namespace archs
{
    const char* get_arch_name()
    {
        #if defined(__x86_64__) || defined(_M_X64)
            return "x86_64";
        #elif defined(__i386__) || defined(_M_IX86)
            return "x86";
        #elif defined(__aarch64__)
            return "aarch64";
        #elif defined(__arm__)
            return "arm";
        #else
            return "unknown";
        #endif
    }

    iarch *get_arch()
    {
        #if defined(__x86_64__) || defined(_M_X64)
            auto mem = reinterpret_cast<iarch*>(placement_kalloc(sizeof(iarch)));
            return new (mem) amd64();
        #else
            return nullptr;
        #endif
    }
}