#pragma once

#include "cpu.hpp"
#include "mem.hpp"
#include <cstring>
#include <iostream>
#include <spdlog/spdlog.h>

struct NES {
    cpumem::Registers cpu_registers;
    cpu::Obj          cpu;

    uint8_t ram_backend[cpumem::RAM_SIZE] = {};
    Memory  ram0;
    Memory  ram1;
    Memory  ram2;
    Memory  ram3;

    uint8_t rom_backend[cpumem::PRG_ROM_SIZE] = {};
    Memory  rom;

    Bus address_bus;
    Bus data_bus;
    Bus write_signal;

    NES()
        : cpu_registers(), cpu(cpu_registers), ram0(ram_backend, 0 * cpumem::RAM_SIZE, cpumem::RAM_SIZE),
          ram1(ram_backend, 1 * cpumem::RAM_SIZE, cpumem::RAM_SIZE),
          ram2(ram_backend, 2 * cpumem::RAM_SIZE, cpumem::RAM_SIZE),
          ram3(ram_backend, 3 * cpumem::RAM_SIZE, cpumem::RAM_SIZE),
          rom(rom_backend, cpumem::PRG_ROM_START, cpumem::PRG_ROM_SIZE), address_bus("address_bus"),
          data_bus("data_bus"), write_signal("write_signal") {

        cpu.address_bus.attach(&cpu, address_bus);
        cpu.data_bus.attach(&cpu, data_bus);
        cpu.write_signal.attach(&cpu, write_signal);

        ram0.address_bus.attach(&ram0, address_bus);
        ram0.data_bus.attach(&ram0, data_bus);
        ram0.write_signal.attach(&ram0, write_signal);
        ram1.address_bus.attach(&ram1, address_bus);
        ram1.data_bus.attach(&ram1, data_bus);
        ram1.write_signal.attach(&ram1, write_signal);
        ram2.address_bus.attach(&ram2, address_bus);
        ram2.data_bus.attach(&ram2, data_bus);
        ram2.write_signal.attach(&ram2, write_signal);
        ram3.address_bus.attach(&ram3, address_bus);
        ram3.data_bus.attach(&ram3, data_bus);
        ram3.write_signal.attach(&ram3, write_signal);

        rom.address_bus.attach(&rom, address_bus);
        rom.data_bus.attach(&rom, data_bus);
        rom.write_signal.attach(&rom, write_signal);

        reset();

        // TODO : Remove this!
        size_t addr = cpu_registers.program_counter;
        for (size_t i = 0; i < 2; i += 1) {
            cpu.address_bus.put(addr);
            spdlog::debug("{:04X}: {:02X}", addr, cpu.data_bus.get());
            addr += 1;
        }
    }

    void save_cartridge(std::ostream os) {
        (void)os;
    }

    void load_cartridge(std::istream is) {
        (void)is;
    }

    void reset() {
        memset(&cpu_registers, 0, sizeof(cpu_registers));
        memset(&ram_backend, 0, sizeof(ram_backend));
        cpu_registers.program_counter = 0xFFFC;
        // cpu_registers.stack_pointer   = 0xFF;
        cpu_registers.processor_status.i = 1;
        cpu.reset();

        // TODO : Remove this!
        {
            uint8_t data[] = {
                0xA2, 0x00, 0xA0, 0x00, 0x8A, 0x99, 0x00, 0x02, 0x48, 0xE8, 0xC8, 0xC0,
                0x10, 0xD0, 0xF5, 0x68, 0x99, 0x00, 0x02, 0xC8, 0xC0, 0x20, 0xD0, 0xF7,
            };
            size_t addr = 0x0600;
            for (auto d : data) {
                cpu.address_bus.put(addr);
                cpu.write_signal.put(true);
                cpu.data_bus.put(d);
                cpu.write_signal.put(false);
                addr += 1;
            }
        }
        {
            uint8_t data[] = {
                0x06,
                0x00,
            };
            size_t addr = cpu_registers.program_counter;
            for (auto d : data) {
                cpu.address_bus.put(addr);
                cpu.write_signal.put(true);
                cpu.data_bus.put(d);
                cpu.write_signal.put(false);
                addr += 1;
            }
        }
    }

    void cycle() {
        cpu.cycle();
    }
};
