#pragma once

#include "chip.hpp"
#include "source/bus.hpp"
#include "source/util.hpp"
#include <cstddef>
#include <cstdint>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

struct Memory {
    uint8_t *backend;
    size_t   address_start;
    size_t   size;
    bool     writeable;

    std::string name;

    BusHarness address_bus;
    BusHarness data_bus;
    BusHarness write_signal;

    static bool bus_level_updated(void *self, size_t bus_level) {
        (void)bus_level;
        Memory *me      = (Memory *)self;
        auto    address = me->address_bus.get();
        auto    data    = me->data_bus.get();
        auto    write   = me->write_signal.get();

        // Address decode
        if ((address < me->address_start) || (address >= (me->address_start + me->size))) {
            return false;
        }

        auto offset = address - me->address_start;
        if (!write) {
            me->data_bus.put(me->backend[offset]);
            spdlog::trace("{} read: {:02X} @ {:04X}", me->name, me->backend[offset], address);
            return true;
        } else if (me->writeable) {
            me->backend[offset] = data;
            spdlog::trace("{} write: {:02X} @ {:04X}", me->name, me->backend[offset], address);
            return true;
        } else {
            spdlog::trace("{} write: {:02X} @ {:04X} NON-WRITEABLE!", me->name, data, address);
            return false;
        }
    }

    Memory(uint8_t *backend, size_t address_start, size_t size, bool writeable = true)
        : backend(backend), address_start(address_start), size(size), writeable(writeable),
          name(fmt::format("memory@{:04X}", address_start)), address_bus(name.c_str(), bus_level_updated),
          data_bus(name.c_str(), bus_level_updated), write_signal(name.c_str(), bus_level_updated) {
        // address_bus.set_callback(bus_level_updated);
        // data_bus.set_callback(bus_level_updated);
        // write_signal.set_callback(bus_level_updated);
        spdlog::debug("Setting up memory @ {:04X} [{:04X}]", address_start, size);
    }
};

namespace cpumem {
enum {
    // On-board resources
    RAM_START       = 0,
    RAM_SIZE        = 0x800,
    ZEROPAGE_START  = 0,
    ZEROPAGE_SIZE   = 0x100,
    STACKPAGE_START = 0x100,
    STACKPAGE_SIZE  = 0x100,
    PPU_REGS_START  = 0x2000,
    PPU_REGS_SIZE   = 0x2000,
    IO_REGS_START   = 0x4000,
    IO_REGS_SIZE    = 0x20,

    // On-cartdrige resources
    CARTRIDGE_ROM_START = 0x4020,
    CARTRIDGE_ROM_SIZE  = 0x1FDF,
    SAVE_RAM_START      = 0x6000,
    SAVE_RAM_SIZE       = 0x2000,
    PRG_ROM_START       = 0x8000,
    PRG_ROM_SIZE        = 0x8000,

    // Vectors
    NMI_VECTOR   = 0xFFFA,
    RESET_VECTOR = 0xFFFC,
    IRQ_VECTOR   = 0xFFFE,
};

struct ProcessorStatus {
    uint8_t n : 1; //!< Negative flag
    uint8_t v : 1; //!< Overflow flag
    uint8_t _ : 1; //!<
    uint8_t b : 1; //!< Break flag
    uint8_t d : 1; //!< Decimal flag
    uint8_t i : 1; //!< Interrupt disable flag
    uint8_t z : 1; //!< Zero flag
    uint8_t c : 1; //!< Carry flag
};

static_assert(sizeof(ProcessorStatus) == 1);

struct Registers {
    uint16_t        program_counter;
    uint8_t         stack_pointer;
    uint8_t         accumulator;
    uint8_t         index_register_x;
    uint8_t         index_register_y;
    ProcessorStatus processor_status;
};

} // namespace cpumem
