#include "cpu.hpp"
#include <optional>
#include <spdlog/spdlog.h>

namespace cpu {
::std::optional<Instruction> Instruction::decode(uint8_t opcode) {
    for (auto instr : instructions) {
        if (instr.opcode == opcode) {
            return instr;
        }
    }
    return ::std::nullopt;
}

void Obj::cycle() {
    switch (state) {
    case State::RESET: {
        // Read reset vector
        buffer[subcycle_counter] = _read(cpumem::RESET_VECTOR + subcycle_counter);
        spdlog::trace("cpu: {} data_bus.get() = {:04X} (@ {:04X})", subcycle_counter, buffer[subcycle_counter],
                      cpumem::RESET_VECTOR + subcycle_counter);
        subcycle_counter += 1;

        if (subcycle_counter == 2) {
            state                     = State::RUN;
            registers.program_counter = u16_des(&buffer[0]); // Jump
            subcycle_counter          = 0;
            spdlog::debug("RESET: Jumping to {:04X}", registers.program_counter);
        }
        break;
    }

    case State::RUN: {
        if (!current_instruction.has_value()) {
            // Grab next instruction
            buffer[0] = _read(registers.program_counter);
            spdlog::trace("cpu: {} Grab next instruction {:02X} @ {:04X}", subcycle_counter, buffer[0],
                          registers.program_counter);
            spdlog::trace("cpu: {} Current registers: A={:02X} X={:02X} Y={:02X} S={:02X} P=[{}]", subcycle_counter,
                          registers.accumulator, registers.index_register_x, registers.index_register_y,
                          registers.stack_pointer, ps_to_string(registers.processor_status));
            current_instruction = Instruction::decode(buffer[0]);
            subcycle_counter    = 1;
            if (current_instruction.has_value()) {
                auto &instr = current_instruction.value();
                // Update the program counter prior to execution
                registers.program_counter += 1;
                // Set initial guess as to how many cycles we'll need
                actual_subcycle_max = instr.cycles;
            } else {
                spdlog::critical("CPU (@{}) OpCode not recognized: {}", cycle_count, buffer[subcycle_counter]);
                state = State::HALT;
            }

        } else if (subcycle_counter < current_instruction.value().num_args) {
            // Read in the instruction arguments
            buffer[subcycle_counter] = _read(registers.program_counter);
            spdlog::trace("cpu: {} Read arg {:02X} @ {:04X}", subcycle_counter, buffer[subcycle_counter],
                          registers.program_counter);
            subcycle_counter += 1;
            // Update the program counter prior to execution
            registers.program_counter += 1;

        } else {
            // Execute opcode callback
            auto       &instr  = current_instruction.value();
            const auto &mnem   = instr.mnemonic;
            std::string bufstr = "";
            for (size_t i = 0; i < instr.num_args; i += 1) {
                bufstr += fmt::format(" {:02X}", buffer[i]);
            }
            trim(bufstr);
            spdlog::trace("cpu: {} Exec {},{} [{}]", subcycle_counter, mnem, addrmode_to_string(instr.addr_mode),
                          bufstr);
            auto ok = (*this.*opcode_funcs[instr.mnemonic])(instr);
            if (!ok) {
                spdlog::error("CPU (@{}) OpCode not implemented or undefined: {},{}", cycle_count, mnem,
                              addrmode_to_string(instr.addr_mode));
                state = State::HALT;
            }
            subcycle_counter += 1;

            // Check if we should go to the next instruction
            if (subcycle_counter > actual_subcycle_max) {
                current_instruction.reset();
                subcycle_counter = 0;
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

uint8_t Obj::_read(uint16_t address) {
    address_bus.put(address);
    return data_bus.get();
}

void Obj::_write(uint8_t data, uint16_t address) {
    address_bus.put(address);
    write_signal.put(true);
    data_bus.put(data);
    write_signal.put(false);
}

void Obj::_push(uint8_t data) {
    _write(data, registers.stack_pointer + cpumem::STACKPAGE_START);
    registers.stack_pointer -= 1;
}

uint8_t Obj::_pop() {
    registers.stack_pointer += 1;
    return _read(registers.stack_pointer + cpumem::STACKPAGE_START);
}

void Obj::_branch(bool do_branch) {
    // Check if this is first time executing
    if (subcycle_counter == current_instruction.value().num_args) {
        // Check the actual branch condition
        if (do_branch) {
            // We're branching
            actual_subcycle_max += 1;
            auto target_program_counter = _addrmode_relative();

            // Check if we should add extra cycle due to page change
            auto next_program_counter_page     = registers.program_counter >> 8;
            auto branched_program_counter_page = target_program_counter >> 8;
            if (next_program_counter_page != branched_program_counter_page) {
                actual_subcycle_max += 1;
            }

            // Do the actual branching
            registers.program_counter = target_program_counter;
        }
    }
}

// // Some instructions have an option to operate directly upon the accumulator. The programmer specifies this by using
// // a special operand value, 'A'. For example:
// // Num arg bytes: 1, num clock cycles: 2
// uint16_t Obj::_addrmode_accumulator() {
//     uint16_t addr = 0;
//     return addr;
// }

// // Immediate addressing allows the programmer to directly specify an 8 bit constant within the instruction. It is
// // indicated by a '#' symbol followed by an numeric expression. For example:
// // Num arg bytes: 2, num clock cycles: 2
// uint16_t Obj::_addrmode_immediate() {
//     uint16_t addr = 0;
//     return addr;
// }

// // For many 6502 instructions the source and destination of the information to be manipulated is implied directly by
// // the function of the instruction itself and no further operand needs to be specified. Operations like 'Clear Carry
// // Flag' (CLC) and 'Return from Subroutine' (RTS) are implicit.
// // Num arg bytes: 2, num clock cycles: differs
// uint16_t Obj::_addrmode_implied() {
//     uint16_t addr = 0;
//     return addr;
// }

// Relative addressing mode is used by branch instructions (e.g. BEQ, BNE, etc.) which contain a signed 8 bit
// relative offset (e.g. -128 to +127) which is added to program counter if the condition is true. As the program
// counter itself is incremented during instruction execution by two the effective address range for the target
// instruction must be with -126 to +129 bytes of the branch.
// Num arg bytes: 2, num clock cycles: 2
uint16_t Obj::_addrmode_relative() {
    return registers.program_counter + (int8_t)u8_des(&buffer[1]);
}

// Instructions using absolute addressing contain a full 16 bit address to identify the target location.
// Num arg bytes: 3, num clock cycles: differs
uint16_t Obj::_addrmode_absolute() {
    return u16_des(&buffer[1]);
}

// An instruction using zero page addressing mode has only an 8 bit address operand. This limits it to addressing
// only the first 256 bytes of memory (e.g. $0000 to $00FF) where the most significant byte of the address is always
// zero. In zero page mode only the least significant byte of the address is held in the instruction making it
// shorter by one byte (important for space saving) and one less memory fetch during execution (important for
// speed).
// An assembler will automatically select zero page addressing mode if the operand evaluates to a zero page address
// and the instruction supports the mode (not all do).
// Num arg bytes: 2, num clock cycles: differs
uint16_t Obj::_addrmode_zero_page() {
    return u8_des(&buffer[1]);
}

// JMP is the only 6502 instruction to support indirection. The instruction contains a 16 bit address which
// identifies the location of the least significant byte of another 16 bit memory address which is the real target
// of the instruction.
// For example if location $0120 contains $FC and location $0121 contains $BA then the instruction JMP ($0120) will
// cause the next instruction execution to occur at $BAFC (e.g. the contents of $0120 and $0121).
// Num arg bytes: 3, num clock cycles: 5
uint16_t Obj::_addrmode_indirect1() {
    // First read the low address byte
    return _read(u16_des(&buffer[1]));
}

uint16_t Obj::_addrmode_indirect2(uint16_t laddr) {
    // Then read the rest of the address
    return laddr | (((uint16_t)_read(u16_des(&buffer[1]) + 1)) << 8);
}

// The address to be accessed by an instruction using X register indexed absolute addressing is computed by taking
// the 16 bit address from the instruction and added the contents of the X register. For example if X contains $92
// then an STA $2000,X instruction will store the accumulator at $2092 (e.g. $2000 + $92).
// Num arg bytes: 3, num clock cycles: differs
uint16_t Obj::_addrmode_absolute_x() {
    return u16_des(&buffer[1]) + registers.index_register_x;
}

// The Y register indexed absolute addressing mode is the same as the previous mode only with the contents of the Y
// register added to the 16 bit address from the instruction.
// Num arg bytes: 3, num clock cycles: differs
uint16_t Obj::_addrmode_absolute_y() {
    return u16_des(&buffer[1]) + registers.index_register_y;
}

// The address to be accessed by an instruction using indexed zero page addressing is calculated by taking the 8 bit
// zero page address from the instruction and adding the current value of the X register to it. For example if the X
// register contains $0F and the instruction LDA $80,X is executed then the accumulator will be loaded from $008F
// (e.g. $80 + $0F => $8F).
// NB: The address calculation wraps around if the sum of the base address and the register exceed $FF. If we repeat
// the last example but with $FF in the X register then the accumulator will be loaded from $007F (e.g. $80 + $FF =>
// $7F) and not $017F.
// Num arg bytes: 2, num clock cycles: differs
uint16_t Obj::_addrmode_zero_page_x() {
    return (u8_des(&buffer[1]) + registers.index_register_x);
}

// The address to be accessed by an instruction using indexed zero page addressing is calculated by taking the 8 bit
// zero page address from the instruction and adding the current value of the Y register to it. This mode can only
// be used with the LDX and STX instructions.
// Num arg bytes: 2, num clock cycles: 4
uint16_t Obj::_addrmode_zero_page_y() {
    return (u8_des(&buffer[1]) + registers.index_register_y);
}

// Indexed indirect addressing is normally used in conjunction with a table of address held on zero page. The
// address of the table is taken from the instruction and the X register added to it (with zero page wrap around) to
// give the location of the least significant byte of the target address.
// Num arg bytes: 2, num clock cycles: 6
uint16_t Obj::_addrmode_x_indirect1() {
    // First read the low address byte
    return _read(u8_des(&buffer[1]) + registers.index_register_x);
}

uint16_t Obj::_addrmode_x_indirect2(uint16_t laddr) {
    // Then read the rest of the address
    return laddr | (((uint16_t)_read(u8_des(&buffer[1]) + registers.index_register_x + ((uint8_t)1))) << 8);
}

// Indirect indexed addressing is the most common indirection mode used on the 6502. In instruction contains the
// zero page location of the least significant byte of 16 bit address. The Y register is dynamically added to this
// value to generated the actual target address for operation.
// Num arg bytes: 2, num clock cycles: differs
uint16_t Obj::_addrmode_indirect_y1() {
    // First read the low address byte
    return _read(u8_des(&buffer[1]));
}

uint16_t Obj::_addrmode_indirect_y2(uint16_t laddr) {
    // Then read the rest of the address
    return (laddr | (((uint16_t)_read(u8_des(&buffer[1]) + 1)) << 8)) + registers.index_register_y;
}

// ADC – Add with Carry
// A,Z,C,N = A+M+C
// This instruction adds the contents of a memory location to the accumulator together with the carry bit. If overflow
// occurs the carry bit is set, this enables multiple byte addition to be performed.
// C: Set if overflow in bit 7
// Z: Set if A = 0
// V: Set if sign bit is incorrect
// N: Set if bit 7 set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
// Absolute,Y 3 4 (+1 if page crossed)
// (Indirect,X) 2 6
// (Indirect),Y 2 5 (+1 if page crossed)
bool Obj::_op_ADC(Instruction &instr) {
    uint16_t ans;
    bool     sign_is_correct = true;
    if (registers.processor_status.d == 0) {
        switch (instr.addr_mode) {
        case IMMEDIATE: {
            auto acc  = registers.accumulator;
            auto oper = u8_des(&buffer[1]);
            ans       = ((uint16_t)acc) + ((uint16_t)oper) + ((uint16_t)registers.processor_status.c);

            registers.accumulator = (uint8_t)ans;
            if ((acc & 0x80) == (oper & 0x80)) {
                sign_is_correct = ((acc & 0x80) == (uint8_t)(ans & 0x80));
            }
            break;
        }
        default: {
            // Unknown addressing mode
            return false;
        }
        }
    } else {
        spdlog::error("CPU (@{}) Decimal mode not implemented for {},{}", cycle_count,
                      current_instruction.value().mnemonic, current_instruction.value().addr_mode);
        return false;
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = (ans > 0x00FF);
        registers.processor_status.z = (registers.accumulator == 0);
        registers.processor_status.v = (!sign_is_correct);
        registers.processor_status.n = ((registers.accumulator & 0x80) != 0);
    }

    return true;
}

// AND – Logical AND
// A,Z,N = A&M
// A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
// Z: Set if A = 0
// N: Set if bit 7 set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
// Absolute,Y 3 4 (+1 if page crossed)
// (Indirect,X) 2 6
// (Indirect),Y 2 5 (+1 if page crossed)
bool Obj::_op_AND(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMMEDIATE: {
        registers.accumulator &= u8_des(&buffer[1]);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.accumulator == 0);
        registers.processor_status.n = ((registers.accumulator & 0x80) != 0);
    }

    return true;
}

// ASL – Arithmetic Shift Left
// A,Z,C,N = M*2 or M,Z,C,N = M*2
// This operation shifts all the bits of the accumulator or memory contents one bit left. Bit 0 is set to 0 and bit 7 is
// placed in the carry flag. The effect of this operation is to multiply the memory contents by 2 (ignoring 2’s
// complement considerations), setting the carry if the result will not fit in 8 bits.
// C	Carry Flag	Set to contents of old bit 7
// Z	Zero Flag	Set if A = 0
// N	Negative Flag	Set if bit 7 of the result is set
// Accumulator 1 2
// Zero Page 2 5
// Zero Page,X 2 6
// Absolute 3 6
// Absolute,X 3 7
bool Obj::_op_ASL(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BCC – Branch if Carry Clear
// If the carry flag is clear then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BCC(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.c == 0);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BCS – Branch if Carry Set
// If the carry flag is set then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BCS(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.c == 1);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BEQ – Branch if Equal
// If the zero flag is set then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BEQ(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.z == 1);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BIT – Bit Test
// A & M, N = M7, V = M6
// This instructions is used to test if one or more bits are set in a target memory location. The mask pattern in A is
// ANDed with the value in memory to set or clear the zero flag, but the result is not kept. Bits 7 and 6 of the value
// from memory are copied into the N and V flags.
// Z: Set if the result if the AND is zero
// V: Set to bit 6 of the memory value
// N: Set to bit 7 of the memory value
// Zero Page 2 3
// Absolute 3 4
bool Obj::_op_BIT(Instruction &instr) {
    uint8_t result;
    switch (instr.addr_mode) {
    case ABSOLUTE: {
        if (subcycle_counter == actual_subcycle_max) {
            result = registers.accumulator & _read(_addrmode_absolute());
        }
        break;
    }
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max) {
            result = registers.accumulator & _read(_addrmode_zero_page());
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (result == 0);
        registers.processor_status.v = ((result & 0x40) != 0);
        registers.processor_status.n = ((result & 0x80) != 0);
    }

    return true;
}

// BMI – Branch if Minus
// If the negative flag is set then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BMI(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.n == 1);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BNE – Branch if Not Equal
// If the zero flag is clear then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BNE(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.z == 0);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BPL – Branch if Positive
// If the negative flag is clear then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BPL(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.n == 0);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BRK – Force Interrupt
// The BRK instruction forces the generation of an interrupt request. The program counter and processor status are
// pushed on the stack then the IRQ interrupt vector at $FFFE/F is loaded into the PC and the break flag in the status
// set to one.
// B: Set to 1
// Implied 1 7
bool Obj::_op_BRK(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BVC – Branch if Overflow Clear
// If the overflow flag is clear then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BVC(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.v == 0);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// BVS – Branch if Overflow Set
// If the overflow flag is set then add the relative displacement to the program counter to cause a branch to a new
// location.
// Relative 2 2 (+1 if branch succeeds, +2 if to a new page)
bool Obj::_op_BVS(Instruction &instr) {
    switch (instr.addr_mode) {
    case RELATIVE: {
        _branch(registers.processor_status.v == 1);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// CLC – Clear Carry Flag
// C = 0
// Set the carry flag to zero.
// C: Set to 0
// Implied 1 2
bool Obj::_op_CLC(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = 0;
    }

    return true;
}

// CLD – Clear Decimal Mode
// D = 0
// Sets the decimal mode flag to zero.
// D: Set to 0
// Implied 1 2
bool Obj::_op_CLD(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.d = 0;
    }

    return true;
}

// CLI – Clear Interrupt Disable
// I = 0
// Clears the interrupt disable flag allowing normal interrupt requests to be serviced.
// I: Set to 0
// Implied 1 2
bool Obj::_op_CLI(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.i = 0;
    }

    return true;
}

// CLV – Clear Overflow Flag
// V = 0
// Clears the overflow flag.
// V: Set to 0
// Implied 1 2
bool Obj::_op_CLV(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.v = 0;
    }

    return true;
}

// CMP – Compare
// Z,C,N = A-M
// This instruction compares the contents of the accumulator with another memory held value and sets the zero and carry
// flags as appropriate.
// C: Set if A >= M
// Z: Set if A = M
// N: Set if bit 7 of the result is set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
// Absolute,Y 3 4 (+1 if page crossed)
// (Indirect,X) 2 6
// (Indirect),Y 2 5 (+1 if page crossed)
bool Obj::_op_CMP(Instruction &instr) {
    uint8_t ans;
    uint8_t M;
    switch (instr.addr_mode) {
    case IMMEDIATE: {
        M = u8_des(&buffer[1]);
        break;
    }
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max) {
            M = _read(_addrmode_zero_page());
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    ans = registers.accumulator - M;

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = (registers.accumulator >= M);
        registers.processor_status.z = (registers.accumulator == M);
        registers.processor_status.n = ((ans & 0x80) != 0);
    }

    return true;
}

// CPX – Compare X Register
// Z,C,N = X-M
// This instruction compares the contents of the X register with another memory held value and sets the zero and carry
// flags as appropriate.
// C: Set if X >= M
// Z: Set if X = M
// N: Set if bit 7 of the result is set
// Immediate 2 2
// Zero Page 2 3
// Absolute 3 4
bool Obj::_op_CPX(Instruction &instr) {
    uint8_t ans;
    uint8_t M;
    switch (instr.addr_mode) {
    case IMMEDIATE: {
        M = u8_des(&buffer[1]);
        break;
    }
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max) {
            M = _read(_addrmode_zero_page());
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    ans = registers.index_register_x - M;

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = (registers.index_register_x >= M);
        registers.processor_status.z = (registers.index_register_x == M);
        registers.processor_status.n = ((ans & 0x80) != 0);
    }

    return true;
}

// CPY – Compare Y Register
// Z,C,N = Y-M
// This instruction compares the contents of the Y register with another memory held value and sets the zero and carry
// flags as appropriate.
// C: Set if Y >= M
// Z: Set if Y = M
// N: Set if bit 7 of the result is set
// Immediate 2 2
// Zero Page 2 3
// Absolute 3 4
bool Obj::_op_CPY(Instruction &instr) {
    uint8_t M;
    switch (instr.addr_mode) {
    case IMMEDIATE: {
        M = u8_des(&buffer[1]);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = (registers.index_register_y >= M);
        registers.processor_status.z = (registers.index_register_y == M);
        registers.processor_status.n = (((registers.index_register_y - M) & 0x80) != 0);
    }

    return true;
}

// DEC – Decrement Memory
// M,Z,N = M-1
// Subtracts one from the value held at a specified memory location setting the zero and negative flags as appropriate.
// Z: Set if result is zero
// N: Set if bit 7 of the result is set
// Zero Page 2 5
// Zero Page,X 2 6
// Absolute 3 6
// Absolute,X 3 7
bool Obj::_op_DEC(Instruction &instr) {
    uint8_t result;
    switch (instr.addr_mode) {
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max - 1) {
            buffer[4] = _read(_addrmode_zero_page());
            buffer[4] -= 1;
        }
        if (subcycle_counter == actual_subcycle_max) {
            _write(buffer[4], _addrmode_zero_page());
            result = buffer[4];
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (result == 0);
        registers.processor_status.n = ((result & 0x80) != 0);
    }

    return true;
}

// DEX – Decrement X Register
// X,Z,N = X-1
// Subtracts one from the X register setting the zero and negative flags as appropriate.
// Z: Set if X is zero
// N: Set if bit 7 of X is set
// Implied 1 2
bool Obj::_op_DEX(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.index_register_x -= 1;
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.index_register_x == 0);
        registers.processor_status.n = ((registers.index_register_x & 0x80) != 0);
    }

    return true;
}

// DEY – Decrement Y Register
// Y,Z,N = Y-1
// Subtracts one from the Y register setting the zero and negative flags as appropriate.
// Z: Set if Y is zero
// N: Set if bit 7 of Y is set
// Implied 1 2
bool Obj::_op_DEY(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.index_register_y -= 1;
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.index_register_y == 0);
        registers.processor_status.n = ((registers.index_register_y & 0x80) != 0);
    }

    return true;
}

// EOR – Exclusive OR
// A,Z,N = A^M
// An exclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
// Z: Set if A = 0
// N: Set if bit 7 set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
// Absolute,Y 3 4 (+1 if page crossed)
// (Indirect,X) 2 6
// (Indirect),Y 2 5 (+1 if page crossed)
bool Obj::_op_EOR(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        // registers.processor_status.z;
        // registers.processor_status.n;
    }

    return true;
}

// INC – Increment Memory
// M,Z,N = M+1
// Adds one to the value held at a specified memory location setting the zero and negative flags as appropriate.
// Z: Set if result is zero
// N: Set if bit 7 of the result is set
// Zero Page 2 5
// Zero Page,X 2 6
// Absolute 3 6
// Absolute,X 3 7
bool Obj::_op_INC(Instruction &instr) {
    switch (instr.addr_mode) {
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max - 1) {
            buffer[3] = _read(_addrmode_zero_page());
            buffer[3] += 1;
        }
        if (subcycle_counter == actual_subcycle_max) {
            _write(buffer[3], _addrmode_zero_page());
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        // registers.processor_status.z;
        // registers.processor_status.n;
    }

    return true;
}

// INX – Increment X Register
// X,Z,N = X+1
// Adds one to the X register setting the zero and negative flags as appropriate.
// Z: Set if X is zero
// N: Set if bit 7 of X is set
// Implied 1 2
bool Obj::_op_INX(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.index_register_x += 1;
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.index_register_x == 0);
        registers.processor_status.n = ((registers.index_register_x & 0x80) != 0);
    }

    return true;
}

// INY – Increment Y Register
// Y,Z,N = Y+1
// Adds one to the Y register setting the zero and negative flags as appropriate.
// Z: Set if Y is zero
// N: Set if bit 7 of Y is set
// Implied 1 2
bool Obj::_op_INY(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.index_register_y += 1;
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.index_register_y == 0);
        registers.processor_status.n = ((registers.index_register_y & 0x80) != 0);
    }

    return true;
}

// JMP – Jump
// Sets the program counter to the address specified by the operand.
// NB: An original 6502 has does not correctly fetch the target address if the indirect vector falls on a page boundary
// (e.g. $xxFF where xx is any value from $00 to $FF). In this case fetches the LSB from $xxFF as expected but takes the
// MSB from $xx00. This is fixed in some later chips like the 65SC02 so for compatibility always ensure the indirect
// vector is not at the end of the page.
// Absolute 3 3
// Indirect  3 5
bool Obj::_op_JMP(Instruction &instr) {
    switch (instr.addr_mode) {
    case ABSOLUTE: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.program_counter = _addrmode_absolute();
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// JSR – Jump to Subroutine
// The JSR instruction pushes the address (minus one) of the return point on to the stack and then sets the program
// counter to the target memory address.
// Absolute 3 6
bool Obj::_op_JSR(Instruction &instr) {
    switch (instr.addr_mode) {
    case ABSOLUTE: {
        if (subcycle_counter == actual_subcycle_max - 1) {
            // Push return address, most signifigant byte must be pushed first
            _push((registers.program_counter - 1) >> 8);
        }
        if (subcycle_counter == actual_subcycle_max) {
            // Push return address, least significant byte
            _push(registers.program_counter - 1);
            registers.program_counter = _addrmode_absolute();
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// LDA – Load Accumulator
// A,Z,N = M
// Loads a byte of memory into the accumulator setting the zero and negative flags as appropriate.
// Z: Set if A = 0
// N: Set if bit 7 of A is set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
// Absolute,Y 3 4 (+1 if page crossed)
// (Indirect,X) 2 6
// (Indirect),Y 2 5 (+1 if page crossed)
bool Obj::_op_LDA(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMMEDIATE: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.accumulator = u8_des(&buffer[1]);
        }
        break;
    }
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.accumulator = _read(_addrmode_zero_page());
        }
        break;
    }
    case ZERO_PAGE_X: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.accumulator = _read(_addrmode_zero_page_x());
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.accumulator == 0);
        registers.processor_status.n = ((registers.accumulator & 0x80) != 0);
    }

    return true;
}

// LDX – Load X Register
// X,Z,N = M
// Loads a byte of memory into the X register setting the zero and negative flags as appropriate.
// Z: Set if X = 0
// N: Set if bit 7 of X is set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,Y 2 4
// Absolute 3 4
// Absolute,Y 3 4 (+1 if page crossed)
bool Obj::_op_LDX(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMMEDIATE: {
        registers.index_register_x = u8_des(&buffer[1]);
        break;
    }
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.index_register_x = _read(_addrmode_zero_page());
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.index_register_x == 0);
        registers.processor_status.n = ((registers.index_register_x & 0x80) != 0);
    }

    return true;
}

// LDY – Load Y Register
// Y,Z,N = M
// Loads a byte of memory into the Y register setting the zero and negative flags as appropriate.
// Z: Set if Y = 0
// N: Set if bit 7 of Y is set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
bool Obj::_op_LDY(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMMEDIATE: {
        registers.index_register_y = u8_des(&buffer[1]);
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
        break;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.index_register_y == 0);
        registers.processor_status.n = ((registers.index_register_y & 0x80) != 0);
    }

    return true;
}

// LSR – Logical Shift Right
// A,C,Z,N = A/2 or M,C,Z,N = M/2
// Each of the bits in A or M is shift one place to the right. The bit that was in bit 0 is shifted into the carry flag.
// Bit 7 is set to zero.
// C: Set to contents of old bit 0
// Z: Set if result = 0
// N: Set if bit 7 of the result is set
// Accumulator 1 2
// Zero Page 2 5
// Zero Page,X 2 6
// Absolute 3 6
// Absolute,X 3 7
bool Obj::_op_LSR(Instruction &instr) {
    uint8_t old, result;
    switch (instr.addr_mode) {
    case ACCUMULATOR: {
        old = registers.accumulator;
        registers.accumulator >>= 1U;
        result = registers.accumulator;
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = ((old & 0x01) != 0);
        registers.processor_status.v = (result == 0);
        registers.processor_status.n = ((result & 0x80) != 0);
    }

    return true;
}

// NOP – No Operation
// The NOP instruction causes no changes to the processor other than the normal incrementing of the program counter to
// the next instruction.
// Implied 1 2
bool Obj::_op_NOP(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// ORA – Logical Inclusive OR
// A,Z,N = A|M
// An inclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
// C: Not affected
// Z: Set if A = 0
// N: Set if bit 7 set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
// Absolute,Y 3 4 (+1 if page crossed)
// (Indirect,X) 2 6
// (Indirect),Y 2 5 (+1 if page crossed)
bool Obj::_op_ORA(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// PHA – Push Accumulator
// Pushes a copy of the accumulator on to the stack.
// Implied 1 3
bool Obj::_op_PHA(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max) {
            _write(registers.accumulator, registers.stack_pointer + cpumem::STACKPAGE_START);
            registers.stack_pointer -= 1;
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// PHP – Push Processor Status
// Pushes a copy of the status flags on to the stack.
// Implied 1 3
bool Obj::_op_PHP(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// PLA – Pull Accumulator
// Pulls an 8 bit value from the stack and into the accumulator. The zero and negative flags are set as appropriate.
// Z: Set if A = 0
// N: Set if bit 7 of A is set
// Implied 1 4
bool Obj::_op_PLA(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.stack_pointer += 1;
            registers.accumulator = _read(registers.stack_pointer + cpumem::STACKPAGE_START);
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.z = (registers.accumulator == 0);
        registers.processor_status.n = ((registers.accumulator & 0x80) != 0);
    }

    return true;
}

// PLP – Pull Processor Status
// Pulls an 8 bit value from the stack and into the processor flags. The flags will take on new states as determined by
// the value pulled.
// C: Set from stack
// Z: Set from stack
// I: Set from stack
// D: Set from stack
// B: Set from stack
// V: Set from stack
// N: Set from stack
// Implied 1 4
bool Obj::_op_PLP(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// ROL – Rotate Left
// Move each of the bits in either A or M one place to the left. Bit 0 is filled with the current value of the carry
// flag whilst the old bit 7 becomes the new carry flag value.
// C: Set to contents of old bit 7
// Z: Set if A = 0
// N: Set if bit 7 of the result is set
// Accumulator 1 2
// Zero Page 2 5
// Zero Page,X 2 6
// Absolute 3 6
// Absolute,X 3 7
bool Obj::_op_ROL(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// ROR – Rotate Right
// Move each of the bits in either A or M one place to the right. Bit 7 is filled with the current value of the carry
// flag whilst the old bit 0 becomes the new carry flag value.
// C: Set to contents of old bit 0
// Z: Set if A = 0
// N: Set if bit 7 of the result is set
// Accumulator 1 2
// Zero Page 2 5
// Zero Page,X 2 6
// Absolute 3 6
// Absolute,X 3 7
bool Obj::_op_ROR(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// RTI – Return from Interrupt
// The RTI instruction is used at the end of an interrupt processing routine. It pulls the processor flags from the
// stack followed by the program counter.
// C: Set from stack
// Z: Set from stack
// I: Set from stack
// D: Set from stack
// B: Set from stack
// V: Set from stack
// N: Set from stack
// Implied 1 6
bool Obj::_op_RTI(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// RTS – Return from Subroutine
// The RTS instruction is used at the end of a subroutine to return to the calling routine. It pulls the program counter
// (minus one) from the stack.
// Implied 1 6
bool Obj::_op_RTS(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max - 1) {
            // Pop return address, least significant byte first
            buffer[3] = _pop();
        }
        if (subcycle_counter == actual_subcycle_max) {
            // Pop return address, most significant byte
            buffer[4]                 = _pop();
            uint16_t addr             = u16_des(&buffer[3]);
            registers.program_counter = addr + 1;
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// SBC – Subtract with Carry
// A,Z,C,N = A-M-(1-C)
// This instruction subtracts the contents of a memory location to the accumulator together with the not of the carry
// bit. If overflow occurs the carry bit is clear, this enables multiple byte subtraction to be performed.
// C: Clear if overflow in bit 7
// Z: Set if A = 0
// V: Set if sign bit is incorrect
// N: Set if bit 7 set
// Immediate 2 2
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 4 (+1 if page crossed)
// Absolute,Y 3 4 (+1 if page crossed)
// (Indirect,X) 2 6
// (Indirect),Y 2 5 (+1 if page crossed)
bool Obj::_op_SBC(Instruction &instr) {
    uint16_t ans;
    bool     sign_is_correct = true;
    if (registers.processor_status.d == 0) {
        switch (instr.addr_mode) {
        case IMMEDIATE: {
            auto acc  = registers.accumulator;
            auto oper = u8_des(&buffer[1]);
            ans       = ((uint16_t)acc) - ((uint16_t)oper) - (1U - ((uint16_t)registers.processor_status.c));

            registers.accumulator = (uint8_t)ans;
            if ((acc & 0x80) == (oper & 0x80)) {
                sign_is_correct = ((acc & 0x80) == (uint8_t)(ans & 0x80));
            }
            break;
        }
        default: {
            // Unknown addressing mode
            return false;
        }
        }
    } else {
        spdlog::error("CPU (@{}) Decimal mode not implemented for {},{}", cycle_count,
                      current_instruction.value().mnemonic, current_instruction.value().addr_mode);
        return false;
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = (ans <= 0x00FF);
        registers.processor_status.z = (registers.accumulator == 0);
        registers.processor_status.v = (!sign_is_correct);
        registers.processor_status.n = ((registers.accumulator & 0x80) != 0);
    }

    return true;
}

// SEC – Set Carry Flag
// C = 1
// Set the carry flag to one.
// C: Set to 1
// Implied 1 2
bool Obj::_op_SEC(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.c = 1;
    }

    return true;
}

// SED – Set Decimal Flag
// D = 1
// Set the decimal mode flag to one.
// D: Set to 1
// Implied 1 2
bool Obj::_op_SED(Instruction &instr) {
    switch (instr.addr_mode) {
    // Comment out here so we can detect anytime we try to enter decimal mode - it is currently not supported
    // case IMPLIED: {
    //     break;
    // }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.d = 1;
    }

    return true;
}

// SEI – Set Interrupt Disable
// I = 1
// Set the interrupt disable flag to one.
// I: Set to 1
// Implied 1 2
bool Obj::_op_SEI(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
        registers.processor_status.i = 1;
    }

    return true;
}

// STA – Store Accumulator
// M = A
// Stores the contents of the accumulator into memory.
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
// Absolute,X 3 5
// Absolute,Y 3 5
// (Indirect,X) 2 6
// (Indirect),Y 2 6
bool Obj::_op_STA(Instruction &instr) {
    switch (instr.addr_mode) {
    case ZERO_PAGE: {
        if (subcycle_counter == actual_subcycle_max) {
            _write(registers.accumulator, _addrmode_zero_page());
        }
        break;
    }
    case ABSOLUTE_Y: {
        if (subcycle_counter == actual_subcycle_max) {
            _write(registers.accumulator, _addrmode_absolute_y());
        }
        break;
    }
    case ZERO_PAGE_X: {
        if (subcycle_counter == actual_subcycle_max) {
            _write(registers.accumulator, _addrmode_zero_page_x());
        }
        break;
    }
    case X_INDIRECT: {
        if (subcycle_counter == actual_subcycle_max - 2) {
            buffer[3] = 0x00;
            buffer[4] = 0x00;
            u16_ser(&buffer[3], _addrmode_x_indirect1());
        }
        if (subcycle_counter == actual_subcycle_max - 1) {
            u16_ser(&buffer[3], _addrmode_x_indirect2(u16_des(&buffer[3])));
        }
        if (subcycle_counter == actual_subcycle_max) {
            _write(registers.accumulator, u16_des(&buffer[3]));
        }
        break;
    }
    case INDIRECT_Y: {
        if (subcycle_counter == actual_subcycle_max - 2) {
            buffer[3] = 0x00;
            buffer[4] = 0x00;
            u16_ser(&buffer[3], _addrmode_indirect_y1());
        }
        if (subcycle_counter == actual_subcycle_max - 1) {
            u16_ser(&buffer[3], _addrmode_indirect_y2(u16_des(&buffer[3])));
        }
        if (subcycle_counter == actual_subcycle_max) {
            _write(registers.accumulator, u16_des(&buffer[3]));
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// STX – Store X Register
// M = X
// Stores the contents of the X register into memory.
// Zero Page 2 3
// Zero Page,Y 2 4
// Absolute 3 4
bool Obj::_op_STX(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// STY – Store Y Register
// M = Y
// Stores the contents of the Y register into memory.
// Zero Page 2 3
// Zero Page,X 2 4
// Absolute 3 4
bool Obj::_op_STY(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// TAX – Transfer Accumulator to X
// X = A
// Copies the current contents of the accumulator into the X register and sets the zero and negative flags as
// appropriate.
// Z: Set if X = 0
// N: Set if bit 7 of X is set
// Implied 1 2
bool Obj::_op_TAX(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// TAY – Transfer Accumulator to Y
// Y = A
// Copies the current contents of the accumulator into the Y register and sets the zero and negative flags as
// appropriate.
// Z: Set if Y = 0
// N: Set if bit 7 of Y is set
// Implied 1 2
bool Obj::_op_TAY(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// TSX – Transfer Stack Pointer to X
// X = S
// Copies the current contents of the stack register into the X register and sets the zero and negative flags as
// appropriate.
// Z: Set if X = 0
// N: Set if bit 7 of X is set
// Implied 1 2
bool Obj::_op_TSX(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// TXA – Transfer X to Accumulator
// A = X
// Copies the current contents of the X register into the accumulator and sets the zero and negative flags as
// appropriate.
// Z: Set if A = 0
// N: Set if bit 7 of A is set
// Implied 1 2
bool Obj::_op_TXA(Instruction &instr) {
    switch (instr.addr_mode) {
    case IMPLIED: {
        if (subcycle_counter == actual_subcycle_max) {
            registers.accumulator = registers.index_register_x;
        }
        break;
    }
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// TXS – Transfer X to Stack Pointer
// S = X
// Copies the current contents of the X register into the stack register.
// Implied 1 2
bool Obj::_op_TXS(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
}

// TYA – Transfer Y to Accumulator
// A = Y
// Copies the current contents of the Y register into the accumulator and sets the zero and negative flags as
// appropriate.
// Z: Set if A = 0
// N: Set if bit 7 of A is set
// Implied 1 2
bool Obj::_op_TYA(Instruction &instr) {
    switch (instr.addr_mode) {
    default: {
        // Unknown addressing mode
        return false;
    }
    }

    // Set flags
    if (subcycle_counter == actual_subcycle_max) {
    }

    return true;
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
