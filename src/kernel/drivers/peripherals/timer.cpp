#include "timer.hpp"
#include "arch/iarch.hpp"
#include "arch/amd64/instructions.hpp"

enum PIT_CHANNEL {
    CHANNEL_0 = 0x40,
    CHANNEL_1 = 0x41,
    CHANNEL_2 = 0x42
};

enum PIT_ACCESS {
    LATCH  = 0x00, // Latch count value
    LOBYTE = 0x10, // Low byte
    HIBYTE = 0x20, // High byte
    LOHI = 0x30    // Low byte then high byte
};

enum PIT_MODE {
    MODE_0 = 0x00, // Interrupt on terminal count
    MODE_1 = 0x02, // One-shot
    MODE_2 = 0x04,  // Rate generator
    MODE_3 = 0x06,  // Square wave generator
    MODE_4 = 0x08,  // Software triggered strobe
    MODE_5 = 0x0A   // Hardware triggered strobe
};

constexpr uint16_t PIT_COMMAND_REG = 0x43;
constexpr uint8_t PIT_BINARY_MODE = 0x00;
constexpr uint8_t PIT_BCD_MODE = 0x01;

// PIT frequency (1.193182 MHz)
constexpr uint32_t PIT_FREQUENCY = 1193182;

peripherals::timer::timer(iarch *arch, uint32_t frequency)
    : arch_(arch), frequency_(frequency), ticks_(0) {

    // Calculate the desired frequncy
    auto div = PIT_FREQUENCY / frequency_;

    // Ensure valid range (1-65535)
    if (div < 1) {
        div = 1;
    } else if (div > 65535) {
        div = 65535;
    }

    // Send command: channel 0, access mode Lo/Hi, Mode 2 (rate gen.), binary mode
    auto cmd = PIT_CHANNEL::CHANNEL_0 | PIT_ACCESS::LOHI | PIT_MODE::MODE_2 | PIT_BINARY_MODE;
    arch->write_byte(PIT_COMMAND_REG, cmd);

    // Send divisor (Lo first, then high byte)
    arch->write_byte(PIT_CHANNEL::CHANNEL_0, div & 0xFF);
    arch->write_byte(PIT_CHANNEL::CHANNEL_0, (div >> 8) & 0xFF);
}

void peripherals::timer::on_timer(const interrupt_t &interrupt) {
    ticks_++;
}

uint32_t peripherals::timer::get_frequency() const {
    return frequency_;
}

uint64_t peripherals::timer::get_ticks() const {
    return ticks_;
}

peripherals::timer &peripherals::add_timer(iarch *arch, uint32_t frequency) {
    static peripherals::timer instance(arch, frequency);
    static timer_handler_t handler(instance, &peripherals::timer::on_timer);

    arch->set_timer_handler(&handler);

    return instance;
}
