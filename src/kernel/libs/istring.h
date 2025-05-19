#ifndef ISTRING_H
#define ISTRING_H

#include "stdint.h"

namespace lib
{
    class istring
    {
    private:
        const char *str_;
        size_t len_;

    public:
        istring(const char *str)
            : str_(str)
        {
        }

        size_t length() const
        {
            return len_;
        }

        const char *c_str() const
        {
            return str_;
        }
    };
}

#endif // ISTRING_H