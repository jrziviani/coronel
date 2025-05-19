#include "pci.h"

constexpr uint8_t MAX_PCI_FUNCTIONS = 8;
constexpr uint8_t MAX_PCI_SLOTS    = 32;

constexpr unsigned PCI_VENDOR_ID     = 0x00;
constexpr unsigned PCI_DEVICE_ID     = 0x02;
constexpr unsigned PCI_SUBCLASS      = 0x0a;
constexpr unsigned PCI_CLASS         = 0x0b;
constexpr unsigned PCI_HEADER_TYPE   = 0x0e;

constexpr unsigned PCI_BAR_0         = 0x10;
constexpr unsigned PCI_BAR_1         = 0x14;
constexpr unsigned PCI_BAR_2         = 0x18;
constexpr unsigned PCI_BAR_3         = 0x1c;
constexpr unsigned PCI_BAR_4         = 0x20;
constexpr unsigned PCI_BAR_5         = 0x24;

constexpr unsigned PCI_SECONDARY_BUS = 0x19;
constexpr unsigned PCI_TYPE_BRIDGE   = 0x604;
constexpr unsigned PCI_NONE          = 0xffff;


// https://wiki.osdev.org/PCI
// Two 32-bit IO locations used: (1) 0xcf8 (config data) and
//                               (2) 0xcfc (config addr)
// CONFIG_ADDRESS: 32-bit register
//            31         24    16       11          8                 0
//      +-------+----------+-----+--------+----------+-----------------+
//      |enable | reserved | bus | device | function | register offset |
//      +-------+----------+-----+--------+----------+-----------------+
//      Notes:
//       - Register offset is the offset in the 256-byte configuration space,
//         which all PCI devices (except host bridges) must provides.
//       - All reads/writes must be 32-bit aligned, so CONFIG_ADDRESS will
//         always end with 0b00 (part of reg offset).
// uint16_t config_read(pci_address_t address, uint8_t offset)
// {
//     constexpr unsigned CONFIG_ADDRESS = 0xcf8;
//     constexpr unsigned CONFIG_DATA    = 0xcfc;
//
//     auto dev_address = (1 << 31) |
//                        (address.bus << 16) |
//                        (address.slot << 11) |
//                        (address.function << 8) |
//                        (offset & 0xfc);
//
//     outl(CONFIG_ADDRESS, dev_address);
//     return static_cast<uint16_t>(inl(CONFIG_DATA) >> ((offset & 2) * 8) & 0xffff);
// }

namespace bus {
    // This function scans all PCI devices in the system and invokes the provided callback
    // for each device found. It starts by scanning the root PCI device and determines
    // whether the system has a single or multiple PCI host controllers.
    void pci::scan_hardware()
    {
        scan_device([](const pci_address_t &addr) {
            (void)addr;
        });
    }

    // This function scans all slots in a given PCI bus. Each slot is checked for devices,
    // and if a device is found, it is further scanned for functions or bridges.
    void pci::scan_bus(pci_function_t callback, int type, uint8_t bus)
    {
        for (uint8_t slot = 0; slot < MAX_PCI_SLOTS; slot++) {
            scan_slot(callback, type, bus, slot);
        }
    }

    // This function scans the root PCI device to determine if the system has a single
    // or multiple PCI host controllers. It then scans the appropriate buses accordingly.
    void pci::scan_device(pci_function_t callback)
    {
        // The 7th bit of PCI HEADER tells whether the device is a multifunction or a single function device.
        // Checking if device 0:0:0 is multifunction means the system has multiple PCI host controllers.

        // Check if the root PCI device is a multifunction device.
        if ((read_byte(pci_address_t(0, 0, 0), PCI_HEADER_TYPE) & 0x80) == 0) {
            // Single PCI host controller, scan the primary bus.
            scan_bus(callback, -1, 0);
            return;
        }

        // Multiple PCI host controllers, scan each controller on its own bus.
        for (uint8_t function = 0; function < MAX_PCI_FUNCTIONS; function++) {
            if (read_word(pci_address_t(0, 0, function), PCI_VENDOR_ID) == PCI_NONE) {
                break;
            }
            scan_bus(callback, -1, function);
        }
    }

    // This function scans a specific slot on a PCI bus. If a device is found in the slot,
    // it scans the device's functions. If the device is a multifunction device, it scans
    // all its functions.
    void pci::scan_slot(pci_function_t cb, int type, uint8_t bus, uint8_t slot)
    {
        pci_address_t addr(bus, slot, 0);
        if (read_word(addr, PCI_VENDOR_ID) == PCI_NONE) {
            return;
        }

        scan_function(cb, type, bus, slot, 0);

        // Check if the device is a multifunction device and scan all its functions.
        if ((read_byte(addr, PCI_HEADER_TYPE) & 0x80) != 0) {
            for (uint8_t function = 1; function < MAX_PCI_FUNCTIONS; function++) {
               if (read_word(pci_address_t(bus, slot, function), PCI_VENDOR_ID) != PCI_NONE) {
                    scan_function(cb, type, bus, slot, function);
                }
            }
        }        
    }

    // This function scans a specific function of a PCI device. If the function matches
    // the specified type or if no type is specified, it invokes the callback with the
    // device's information. If the device is a bridge, it scans the secondary bus.
    void pci::scan_function(pci_function_t cb, int type, uint8_t bus, uint8_t slot, uint8_t function)
    {
        pci_address_t addr(bus, slot, function);
        if (type == -1 || type == get_type(addr)) {
            pci_info_t info {
                addr,
                read_byte(addr, PCI_CLASS),
                read_byte(addr, PCI_SUBCLASS),

                read_word(addr, PCI_VENDOR_ID),
                read_word(addr, PCI_DEVICE_ID),

                read_dword(addr, PCI_BAR_0),
                read_dword(addr, PCI_BAR_1),
                read_dword(addr, PCI_BAR_2),
                read_dword(addr, PCI_BAR_3),
                read_dword(addr, PCI_BAR_4),
                read_dword(addr, PCI_BAR_5)
            };
            
            arch_->get_video()->prints(" > PCI ");
            arch_->get_video()->printd(info.address.bus);
            arch_->get_video()->prints(":");
            arch_->get_video()->printd(info.address.slot);
            arch_->get_video()->prints(":");
            arch_->get_video()->printd(info.address.function);
            arch_->get_video()->prints(" - 0x");
            arch_->get_video()->printx(info.vendor);
            arch_->get_video()->prints(" 0x");
            arch_->get_video()->printx(info.device);
            arch_->get_video()->prints(" - 0x");
            arch_->get_video()->printx(info.klass);
            arch_->get_video()->prints(" 0x");
            arch_->get_video()->printx(info.subclass);
            arch_->get_video()->prints("\n");

            if (info.klass == 0x1 && info.subclass == 0x6) {
                arch_->get_video()->prints("   ^-- Bars: 0x");
                arch_->get_video()->printx(info.bars[0]);
                arch_->get_video()->prints(" 0x");
                arch_->get_video()->printx(info.bars[1]);
                arch_->get_video()->prints(" 0x");
                arch_->get_video()->printx(info.bars[2]);
                arch_->get_video()->prints(" 0x");
                arch_->get_video()->printx(info.bars[3]);
                arch_->get_video()->prints(" 0x");
                arch_->get_video()->printx(info.bars[4]);
                arch_->get_video()->prints(" 0x");
                arch_->get_video()->printx(info.bars[5]);
                arch_->get_video()->prints("\n");
            }
        }

        // if the device is a bridge, scan the secondary bus
        if (type == PCI_TYPE_BRIDGE) {
            uint8_t secondary_bus = read_byte(addr, PCI_SECONDARY_BUS);
            if (secondary_bus != 0) {
                scan_bus(cb, type, secondary_bus);
            }
        }
    }

    // This function retrieves the type of a PCI device by combining its class and subclass
    // values into a single 16-bit value.
    uint16_t pci::get_type(pci_address_t addr) const
    {
        return static_cast<uint16_t>((read_byte(addr, PCI_CLASS) << 8u)
                                    | read_byte(addr, PCI_SUBCLASS));
    }
}