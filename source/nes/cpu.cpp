#include "cpu.hpp"
#include <spdlog/spdlog.h>

namespace cpu {
std::optional<Instruction> Instruction::decode(uint8_t opcode) {
    for (auto instr : instructions) {
        if (instr.opcode == opcode) {
            return instr;
        }
    }
    return std::nullopt;
}

void Obj::cycle() {
    switch (state) {
    case State::RESET: {
        // Read reset vector
        address_bus.put(registers.program_counter + subcycle_counter);
        buffer[subcycle_counter] = data_bus.get();
        spdlog::trace("cpu: {} data_bus.get() = {:04X} (@ {:04X})", subcycle_counter, buffer[subcycle_counter],
                      registers.program_counter + subcycle_counter);
        subcycle_counter += 1;
        if (subcycle_counter == 2) {
            state                     = State::RUN;
            registers.program_counter = (buffer[0] << 8) | buffer[1]; // Jump
            subcycle_counter          = 0;
            spdlog::debug("RESET: Jumping to {:04X}", registers.program_counter);
        }
        break;
    }
    case State::RUN: {
        if (!current_instruction.has_value()) {
            // Grab next instruction
            address_bus.put(registers.program_counter);
            buffer[subcycle_counter] = data_bus.get();
            current_instruction      = Instruction::decode(buffer[subcycle_counter]);
            subcycle_counter += 1;
            if (!current_instruction.has_value()) {
                spdlog::critical("CPU (@{}) OpCode not recognized: {}", cycle_count, buffer[subcycle_counter]);
                state = State::HALT;
            }
        } else if (subcycle_counter < current_instruction.value().num_args) {
            // Read in the instruction arguments
            address_bus.put(registers.program_counter + subcycle_counter);
            buffer[subcycle_counter] = data_bus.get();
            subcycle_counter += 1;
        } else {
            auto       &instr = current_instruction.value();
            const auto &mnem  = instr.mnemonic;
            if (mnem == ADC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == AND) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == ASL) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BCC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BCS) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BEQ) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BIT) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BMI) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BNE) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BPL) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BRK) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BVC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == BVS) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == CLC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == CLD) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == CLI) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == CLV) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == CMP) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == CPX) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == CPY) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == DEC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == DEX) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == DEY) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == EOR) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == INC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == INX) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == INY) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == JMP) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == JSR) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == LDA) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == LDX) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == LDY) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == LSR) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == NOP) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == ORA) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == PHA) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == PHP) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == PLA) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == PLP) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == ROL) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == ROR) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == RTI) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == RTS) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == SBC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == SEC) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == SED) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == SEI) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == STA) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == STX) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == STY) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == TAX) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == TAY) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == TSX) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == TXA) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == TXS) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else if (mnem == TYA) {
                spdlog::error("CPU (@{}) OpCode not yet implemented: {}", cycle_count, mnem);
                state = State::HALT;
            } else {
                spdlog::critical("CPU (@{}) OpCode not recognized: {}", cycle_count, mnem);
                state = State::HALT;
            }
        }
        break;
    }
    case State::HALT:
        [[fallthrough]];
    default: {
        break;
    }
    }
    cycle_count += 1;
}
} // namespace cpu

#ifdef TEST
#include "source/util.hpp"
#include <algorithm>
#include <catch2/catch.hpp>
#include <fmt/core.h>
#include <vector>
TEST_CASE("instruction checks", "[cpu]") {
    SECTION("unique opcodes") {
        // Collect all opcodes
        std::vector<decltype(cpu::Instruction::opcode)> v;
        for (auto instr : cpu::instructions) {
            v.push_back(instr.opcode);
        }

        // Sort and find duplicates
        std::sort(v.begin(), v.end());
        REQUIRE(std::adjacent_find(v.begin(), v.end()) == v.end());
    }
}
#endif
