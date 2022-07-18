#pragma once

#include "cartridge.hpp"
#include "cpu.hpp"
#include "mem.hpp"
#include "source/globals.hpp"
#include <cstring>
#include <spdlog/spdlog.h>

// TODO : Remove this!
// static uint8_t program[] = {
//     // clang-format off
//     //   LDX #$00
//     0xA2, 0x00, // 0x0600
//     //   LDY #$00
//     0xA0, 0x00, // 0x0602
//     // firstloop:
//     //   TXA
//     0x8A, // 0x0604
//     //   STA $0200,Y
//     0x99, 0x00, 0x02,  // 0x0605
//     //   PHA
//     0x48, // 0x0608
//     //   INX
//     0xE8, // 0x0609
//     //   INY
//     0xC8, // 0x060A
//     //   CPY #$10
//     0xC0, 0x10, // 0x060B
//     //   BNE firstloop ;loop until Y is $10
//     0xD0, 0xF5, // 0x060D
//     // secondloop:
//     //   PLA
//     0x68, // 0x060F
//     //   STA $0200,Y
//     0x99, 0x00, 0x02, // 0x0610
//     //   INY
//     0xC8, // 0x0613
//     //   CPY #$20      ;loop until Y is $20
//     0xC0, 0x20, // 0x0614
//     //   BNE secondloop
//     0xD0, 0xF7, // 0x0616
//     // clang-format on
// };

// static uint8_t program[] = {
//     0x20, 0x06, 0x06, 0x20, 0x38, 0x06, 0x20, 0x0D, 0x06, 0x20, 0x2A, 0x06, 0x60, 0xA9, 0x02, 0x85, 0x02, 0xA9, 0x04,
//     0x85, 0x03, 0xA9, 0x11, 0x85, 0x10, 0xA9, 0x10, 0x85, 0x12, 0xA9, 0x0F, 0x85, 0x14, 0xA9, 0x04, 0x85, 0x11, 0x85,
//     0x13, 0x85, 0x15, 0x60, 0xA5, 0xFE, 0x85, 0x00, 0xA5, 0xFE, 0x29, 0x03, 0x18, 0x69, 0x02, 0x85, 0x01, 0x60, 0x20,
//     0x4D, 0x06, 0x20, 0x8D, 0x06, 0x20, 0xC3, 0x06, 0x20, 0x19, 0x07, 0x20, 0x20, 0x07, 0x20, 0x2D, 0x07, 0x4C, 0x38,
//     0x06, 0xA5, 0xFF, 0xC9, 0x77, 0xF0, 0x0D, 0xC9, 0x64, 0xF0, 0x14, 0xC9, 0x73, 0xF0, 0x1B, 0xC9, 0x61, 0xF0, 0x22,
//     0x60, 0xA9, 0x04, 0x24, 0x02, 0xD0, 0x26, 0xA9, 0x01, 0x85, 0x02, 0x60, 0xA9, 0x08, 0x24, 0x02, 0xD0, 0x1B, 0xA9,
//     0x02, 0x85, 0x02, 0x60, 0xA9, 0x01, 0x24, 0x02, 0xD0, 0x10, 0xA9, 0x04, 0x85, 0x02, 0x60, 0xA9, 0x02, 0x24, 0x02,
//     0xD0, 0x05, 0xA9, 0x08, 0x85, 0x02, 0x60, 0x60, 0x20, 0x94, 0x06, 0x20, 0xA8, 0x06, 0x60, 0xA5, 0x00, 0xC5, 0x10,
//     0xD0, 0x0D, 0xA5, 0x01, 0xC5, 0x11, 0xD0, 0x07, 0xE6, 0x03, 0xE6, 0x03, 0x20, 0x2A, 0x06, 0x60, 0xA2, 0x02, 0xB5,
//     0x10, 0xC5, 0x10, 0xD0, 0x06, 0xB5, 0x11, 0xC5, 0x11, 0xF0, 0x09, 0xE8, 0xE8, 0xE4, 0x03, 0xF0, 0x06, 0x4C, 0xAA,
//     0x06, 0x4C, 0x35, 0x07, 0x60, 0xA6, 0x03, 0xCA, 0x8A, 0xB5, 0x10, 0x95, 0x12, 0xCA, 0x10, 0xF9, 0xA5, 0x02, 0x4A,
//     0xB0, 0x09, 0x4A, 0xB0, 0x19, 0x4A, 0xB0, 0x1F, 0x4A, 0xB0, 0x2F, 0xA5, 0x10, 0x38, 0xE9, 0x20, 0x85, 0x10, 0x90,
//     0x01, 0x60, 0xC6, 0x11, 0xA9, 0x01, 0xC5, 0x11, 0xF0, 0x28, 0x60, 0xE6, 0x10, 0xA9, 0x1F, 0x24, 0x10, 0xF0, 0x1F,
//     0x60, 0xA5, 0x10, 0x18, 0x69, 0x20, 0x85, 0x10, 0xB0, 0x01, 0x60, 0xE6, 0x11, 0xA9, 0x06, 0xC5, 0x11, 0xF0, 0x0C,
//     0x60, 0xC6, 0x10, 0xA5, 0x10, 0x29, 0x1F, 0xC9, 0x1F, 0xF0, 0x01, 0x60, 0x4C, 0x35, 0x07, 0xA0, 0x00, 0xA5, 0xFE,
//     0x91, 0x00, 0x60, 0xA6, 0x03, 0xA9, 0x00, 0x81, 0x10, 0xA2, 0x00, 0xA9, 0x01, 0x81, 0x10, 0x60, 0xA2, 0x00, 0xEA,
//     0xEA, 0xCA, 0xD0, 0xFB, 0x60,
// };

// uint8_t reset_vector[] = {
//     // clang-format off
//     0x00, 0x06,
//     // clang-format on
// };

struct NES {
    // enum class CartridgeType {
    //     iNES,
    // };

    cpumem::Registers cpu_registers;
    cpu::Obj          cpu;

    uint8_t ram_backend[cpumem::RAM_SIZE] = {};

    // uint8_t rom_backend[cpumem::PRG_ROM_SIZE] = {};

    std::vector<Memory>        onboard_memories;
    std::unique_ptr<Cartridge> cartridge;

    Bus address_bus;
    Bus data_bus;
    Bus write_signal;

    NES()
        : cpu_registers(), cpu(cpu_registers), address_bus("address_bus"), data_bus("data_bus"),
          write_signal("write_signal") {

        this->cpu.address_bus.attach(&cpu, address_bus);
        this->cpu.data_bus.attach(&cpu, data_bus);
        this->cpu.write_signal.attach(&cpu, write_signal);

        this->onboard_memories.push_back({ram_backend, 0 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});
        this->onboard_memories.push_back({ram_backend, 1 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});
        this->onboard_memories.push_back({ram_backend, 2 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});
        this->onboard_memories.push_back({ram_backend, 3 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});
        // this->onboard_memories.push_back({rom_backend, cpumem::PRG_ROM_START, cpumem::PRG_ROM_SIZE});

        for (size_t i = 0; i < this->onboard_memories.size(); i += 1) {
            auto &memory = this->onboard_memories[i];
            memory.address_bus.attach(&memory, address_bus);
            memory.data_bus.attach(&memory, data_bus);
            memory.write_signal.attach(&memory, write_signal);
        }
        // for (auto &memory : this->onboard_memories) {
        //     memory.address_bus.attach(&memory, address_bus);
        //     memory.data_bus.attach(&memory, data_bus);
        //     memory.write_signal.attach(&memory, write_signal);
        // }

        reset();

        // TODO : Remove this!
        // Dump program
        // print_mem(0x0600, ARR_LEN(program));
    }

    // void save_cartridge(std::ostream os) {
    //     (void)os;
    // }

    void load_cartridge(std::vector<uint8_t> raw) {
        if (raw.size() < 16) {
            spdlog::error("iNES cartridge too small, size is: {}", raw.size());
            return;
        }

        // First check magic header
        if (raw[0] != 'N' || raw[1] != 'E' || raw[2] != 'S' || raw[3] != 0x1A) {
            spdlog::error("Wrong magic in iNES cartridge: {:02X} {:02X} {:02X} {:02X}", raw[0], raw[1], raw[2], raw[3]);
            return;
        }

        size_t rom_size = raw[4] * 16 * 1024;
        // size_t vrom_size     = raw[5] * 8 * 1024;
        auto control_byte1 = raw[6];
        auto control_byte2 = raw[7];

        auto vertical_mirroring = ((control_byte1 & 0x01) != 0);
        // auto has_battery_ram    = ((control_byte1 & 0x02) != 0);
        auto has_trainer = ((control_byte1 & 0x04) != 0);
        auto four_screen = ((control_byte1 & 0x08) != 0);

        auto ines_format_version = (control_byte2 & 0x0F);

        if (ines_format_version != 0) {
            spdlog::error("Unrecognized iNES cartridge format: {}", ines_format_version);
            return;
        }

        // We're probably good to go at this point!

        spdlog::debug("Allocating a new cartridge");
        this->cartridge = std::make_unique<Cartridge>(); // Throw old cartridge
        auto &cartridge = *this->cartridge.get();

        cartridge.mapper = (control_byte1 >> 4) | (control_byte2 & 0xF0);

        if (four_screen) {
            cartridge.mirroring = Cartridge::Mirroring::FOUR_SCREEN;
        } else if (vertical_mirroring) {
            cartridge.mirroring = Cartridge::Mirroring::VERTICAL;
        } else {
            cartridge.mirroring = Cartridge::Mirroring::HORIZONTAL;
        }

        spdlog::debug("Allocating ROM in cartridge, size: {}", rom_size);
        cartridge.backends.push_back(std::make_unique<uint8_t[]>(rom_size));
        uint8_t *rom_backend = cartridge.backends[cartridge.backends.size() - 1].get();
        cartridge.memories.push_back({rom_backend, cpumem::PRG_ROM_START, rom_size});
        if (rom_size == (cpumem::PRG_ROM_SIZE / 2)) {
            // If we only got 16KiBs of ROM we need to mirror upper part
            cartridge.memories.push_back({rom_backend, cpumem::PRG_ROM_START + rom_size, rom_size});
        }

        // cartridge.backends.push_back(std::make_unique<uint8_t[]>(vrom_size));
        // uint8_t *vrom_backend = cartridge.backends[cartridge.backends.size() - 1].get();

        spdlog::debug("Attaching cartridge");
        for (size_t i = 0; i < cartridge.memories.size(); i += 1) {
            auto &memory = cartridge.memories[i];
            memory.address_bus.attach(&memory, address_bus);
            memory.data_bus.attach(&memory, data_bus);
            memory.write_signal.attach(&memory, write_signal);
        }

        // Read in the data
        auto rom_start = 16 + (has_trainer ? 512 : 0);
        spdlog::debug("Reading in cartridge contents");
        // auto vrom_start = rom_start + rom_size;
        cpu_write_mem(cpumem::PRG_ROM_START, &raw[rom_start], rom_size);

        reset();
    }

    void cpu_write_mem(size_t addr, uint8_t *buf, size_t size) {
        for (size_t i = 0; i < size; i += 1) {
            cpu.address_bus.put(addr + i);
            cpu.write_signal.put(true);
            cpu.data_bus.put(buf[i]);
            cpu.write_signal.put(false);
        }
    }

    std::vector<uint8_t> dump_mem(size_t addr, size_t size) {
        std::vector<uint8_t> vec;
        for (size_t i = 0; i < size; i += 1) {
            auto data = cpu._read(addr + i);
            vec.push_back(data);
        }
        return vec;
    }

    void print_mem(size_t addr, size_t size, size_t stride = 16) {
        size_t      count = 0;
        std::string dump  = fmt::format("Dump @ {:04X}:", addr);
        for (uint8_t data : dump_mem(addr, size)) {
            if (count % stride == 0) {
                dump += fmt::format("\n{:04X}: ", addr + count);
            }

            if (count % 8 == 0) {
                dump += " ";
            }

            dump += fmt::format("{:02X} ", data);
            count += 1;
        }
        spdlog::debug(dump);
    }

    void reset() {
        spdlog::debug("Resetting NES");
        memset(&cpu_registers, 0, sizeof(cpu_registers));
        memset(&ram_backend, 0, sizeof(ram_backend));
        // cpu_registers.program_counter = 0xFFFC;
        cpu_registers.stack_pointer      = 0xFF;
        cpu_registers.processor_status.i = 1;
        cpu.reset();

        // TODO : Remove this!
        // cpu_write_mem(0x0600, program, ARR_LEN(program));
        // cpu_write_mem(cpumem::RESET_VECTOR, reset_vector, ARR_LEN(reset_vector));
    }

    void cycle() {
        cpu.cycle();
        total_cycle_count += 1;
    }
};
