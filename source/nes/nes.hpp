#pragma once

#include "cartridge.hpp"
#include "cpu.hpp"
#include "mem.hpp"
#include "source/globals.hpp"
#include "source/util.hpp"
#include <cstring>
#include <spdlog/spdlog.h>

struct NES {
    std::function<int()> get_random = nullptr;

    enum class Variant {
        NTSC_2C02, // NTSC (2C02)
        PAL_2C07,  // PAL (2C07)
        Dendy,     // Dendy
        RGB_2C03,  // RGB (2C03)
        RGB_Vs_4,  // RGB (Vs. 4)
        RGB_2C05,  // RGB (2C05)
        _MAX
    };

    Variant variant;

    constexpr uint64_t master_clock_frequency() const {
        // From https://www.nesdev.org/wiki/Cycle_reference_chart
        // NTSC (2C02): 21.477272 MHz ± 40 Hz, 236.25 MHz ÷ 11 by definition
        // PAL (2C07): 26.601712 MHz ± 50 Hz 26.6017125 MHz by definition
        // Dendy; Like PAL
        // RGB (2C03), RGB (Vs. 4), RGB (2C05): Like NTSC

        if (variant == Variant::PAL_2C07 || variant == Variant::Dendy) {
            return 26'601'712ULL;
        } else {
            return 21'477'272ULL;
        }
    }

    cpu::Obj cpu;

    uint8_t ram_backend[cpumem::RAM_SIZE] = {};

    std::vector<Memory>        onboard_memories;
    std::unique_ptr<Cartridge> cartridge;

    Bus address_bus;
    Bus data_bus;
    Bus write_signal;

    NES() : address_bus("address_bus"), data_bus("data_bus"), write_signal("write_signal") {
    }

    void init() {
        address_bus.detach_all();
        data_bus.detach_all();
        write_signal.detach_all();

        this->cpu.address_bus.attach(&cpu, address_bus);
        this->cpu.data_bus.attach(&cpu, data_bus);
        this->cpu.write_signal.attach(&cpu, write_signal);

        this->onboard_memories.push_back({ram_backend, 0 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});
        this->onboard_memories.push_back({ram_backend, 1 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});
        this->onboard_memories.push_back({ram_backend, 2 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});
        this->onboard_memories.push_back({ram_backend, 3 * cpumem::RAM_SIZE, cpumem::RAM_SIZE});

        for (size_t i = 0; i < this->onboard_memories.size(); i += 1) {
            auto &memory = this->onboard_memories[i];
            memory.address_bus.attach(&memory, address_bus);
            memory.data_bus.attach(&memory, data_bus);
            memory.write_signal.attach(&memory, write_signal);
        }
    }

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
        init();

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
        memset(&ram_backend, 0, sizeof(ram_backend));
        cpu.reset();
    }

    void main_cycle() {
        cpu.cycle();
        total_cycle_count += 1;
    }
};
