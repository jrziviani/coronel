#ifndef TIMER_HPP
#define TIMER_HPP

class iarch;

#include "libs/stdint.hpp"
#include "libs/functional.hpp"

namespace peripherals
{
    class timer
    {
    private:
        iarch *arch_;
        uint32_t frequency_;
        uint64_t ticks_;

    public:
        timer(iarch *arch, uint32_t frequency);

        void on_timer(const interrupt_t &interrupt);

        uint32_t get_frequency() const;
        uint64_t get_ticks() const;
    };

    timer &add_timer(iarch *arch, uint32_t frequency);
}

using timer_handler_t = lib::interrupt_handler_type<peripherals::timer>;

#endif // TIMER_HPP