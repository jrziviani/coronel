#ifndef PCI_HPP
#define PCI_HPP

#include "libs/stdint.hpp"
#include "arch/iarch.hpp"

namespace bus
{
    constexpr unsigned PCI_ADDRESS_PORT = 0xcf8;
    constexpr unsigned PCI_VALUE_PORT   = 0xcfc;

    struct pci_address_t {
        uint8_t bus;
        uint8_t slot;
        uint8_t function;

        pci_address_t(uint8_t bus, uint8_t slot, uint8_t function)
            : bus(bus), slot(slot), function(function) {}

        uint32_t get_address(uint8_t field) const {
            return static_cast<uint32_t>(0x80000000)
                | (bus << 16u)
                | (slot << 11u)
                | (function << 8u)
                | (field & 0xfc);
        }
    };

    struct pci_info_t {
        pci_address_t address;

        uint8_t klass;
        uint8_t subclass;
        uint16_t vendor;
        uint16_t device;
        uint32_t bars[6];

        uint64_t hash() const {
            return (static_cast<uint64_t>(vendor) << 32u) | device;
        }

        bool operator==(const pci_info_t& other) const {
            return address.bus == other.address.bus
                && address.slot == other.address.slot
                && address.function == other.address.function;
        }
    };

    using pci_function_t = void (*)(const pci_address_t &addr);

    class pci
    {
        iarch *arch_;

    public:
        pci(iarch *arch) : arch_(arch) {}
        void scan_hardware();

    protected:
        uint8_t read_byte(const pci_address_t &addr, uint16_t field) const
        {
            return arch_->read_byte(addr.get_address(field), PCI_ADDRESS_PORT, PCI_VALUE_PORT + (field & 0x3));
        }

        uint16_t read_word(const pci_address_t &addr, uint16_t field) const
        {
            return arch_->read_word(addr.get_address(field), PCI_ADDRESS_PORT, PCI_VALUE_PORT + (field & 0x2));
        }

        uint32_t read_dword(const pci_address_t &addr, uint16_t field) const
        {
            return arch_->read_dword(addr.get_address(field), PCI_ADDRESS_PORT, PCI_VALUE_PORT);
        }

        void write_byte(const pci_address_t &addr, uint16_t field, uint8_t value) const
        {
            arch_->write_byte(addr.get_address(field), PCI_ADDRESS_PORT, PCI_VALUE_PORT, value);
        }

        void write_word(const pci_address_t &addr, uint16_t field, uint16_t value) const
        {
            arch_->write_word(addr.get_address(field), PCI_ADDRESS_PORT, PCI_VALUE_PORT, value);
        }

        void write_dword(const pci_address_t &addr, uint16_t field, uint32_t value) const
        {
            arch_->write_dword(addr.get_address(field), PCI_ADDRESS_PORT, PCI_VALUE_PORT, value);
        }

    private:
        static uint32_t device(uint32_t bus, uint32_t slot, uint32_t function) {
            return static_cast<uint32_t>((bus << 16u) | (slot << 8u) | function);
        }

        void scan_device(pci_function_t cb);
        void scan_bus(pci_function_t cb, int type, uint8_t bus);
        void scan_slot(pci_function_t cb, int type, uint8_t bus, uint8_t slot);
        void scan_function(pci_function_t cb, int type, uint8_t bus, uint8_t slot, uint8_t function);

        uint16_t get_type(pci_address_t addr) const;
    };

    inline pci &get_pci(iarch *arch)
    {
        static pci instance(arch);
        return instance;
    }
}

#endif // PCI_HPP