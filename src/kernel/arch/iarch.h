#ifndef IARCH_H
#define IARCH_H

#include "libs/stdint.h"
#include "drivers/peripherals/keyboard.h"
#include "iprotected_mode.h"

class iarch
{
public:
    virtual void cpu_halt() = 0;

    virtual iprotected_mode *get_video() = 0;

    virtual void set_keyboard_handler(const keyboard_handler_t *handler) = 0;

    virtual uint8_t read_byte(uint32_t addr, uint16_t addr_port, uint16_t value_port) const = 0;
    virtual uint16_t read_word(uint32_t addr, uint16_t addr_port, uint16_t value_port) const = 0;
    virtual uint32_t read_dword(uint32_t addr, uint16_t addr_port, uint16_t value_port) const = 0;
    virtual void write_byte(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint8_t value) const = 0;
    virtual void write_word(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint16_t value) const = 0;
    virtual void write_dword(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint32_t value) const = 0;
};

#endif // IARCH_H