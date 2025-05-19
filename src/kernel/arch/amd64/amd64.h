#ifndef AMD64_H
#define AMD64_H

#include <arch/iarch.h>
#include "video/protected_mode.h"

class amd64 : public iarch
{
private:
    protected_mode video_;

public:
    amd64() = default;

    void cpu_halt() override;

    iprotected_mode *get_video() override
    {
        return &video_;
    }
};

#endif // AMD64_H