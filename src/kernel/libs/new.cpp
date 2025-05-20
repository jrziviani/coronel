#include "new.hpp"

void *operator new(size_t size) throw()
{
    (void)size;
    return nullptr;
}

void *operator new[](size_t size) throw()
{
    (void)size;
    return nullptr;
}

void operator delete(void *p)
{
    (void)p;
}

void operator delete[](void *p)
{
    (void)p;
}

void operator delete(void*, void*) noexcept
{
}

void operator delete[](void*, void*) noexcept
{
}

void operator delete(void*, unsigned long) noexcept
{
}

void operator delete[](void*, unsigned long) noexcept
{
}