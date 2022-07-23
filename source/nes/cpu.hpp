#pragma once

#include "chip.hpp"
#include "mem.hpp"
#include "source/bus.hpp"
#include <cstdint>
#include <fmt/core.h>
#include <map>
#include <optional>

namespace cpu {
// Mnemonics
enum class OpCode {
    ADC = 0,
    AND = 1,
    ASL = 2,
    BCC = 3,
    BCS = 4,
    BEQ = 5,
    BIT = 6,
    BMI = 7,
    BNE = 8,
    BPL = 9,
    BRK = 10,
    BVC = 11,
    BVS = 12,
    CLC = 13,
    CLD = 14,
    CLI = 15,
    CLV = 16,
    CMP = 17,
    CPX = 18,
    CPY = 19,
    DEC = 20,
    DEX = 21,
    DEY = 22,
    EOR = 23,
    INC = 24,
    INX = 25,
    INY = 26,
    JMP = 27,
    JSR = 28,
    LDA = 29,
    LDX = 30,
    LDY = 31,
    LSR = 32,
    NOP = 33,
    ORA = 34,
    PHA = 35,
    PHP = 36,
    PLA = 37,
    PLP = 38,
    ROL = 39,
    ROR = 40,
    RTI = 41,
    RTS = 42,
    SBC = 43,
    SEC = 44,
    SED = 45,
    SEI = 46,
    STA = 47,
    STX = 48,
    STY = 49,
    TAX = 50,
    TAY = 51,
    TSX = 52,
    TXA = 53,
    TXS = 54,
    TYA = 55,
    _NUM,
};

inline constexpr const char *op_code_names[] = {
    "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS", "CLC",
    "CLD", "CLI", "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP",
    "JSR", "LDA", "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL", "ROR", "RTI",
    "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY", "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",
};

inline const char *opcode_to_string(OpCode opcode) {
    return op_code_names[(size_t)opcode];
}

// inline constexpr const char *ADC = "ADC";
// inline constexpr const char *AND = "AND";
// inline constexpr const char *ASL = "ASL";
// inline constexpr const char *BCC = "BCC";
// inline constexpr const char *BCS = "BCS";
// inline constexpr const char *BEQ = "BEQ";
// inline constexpr const char *BIT = "BIT";
// inline constexpr const char *BMI = "BMI";
// inline constexpr const char *BNE = "BNE";
// inline constexpr const char *BPL = "BPL";
// inline constexpr const char *BRK = "BRK";
// inline constexpr const char *BVC = "BVC";
// inline constexpr const char *BVS = "BVS";
// inline constexpr const char *CLC = "CLC";
// inline constexpr const char *CLD = "CLD";
// inline constexpr const char *CLI = "CLI";
// inline constexpr const char *CLV = "CLV";
// inline constexpr const char *CMP = "CMP";
// inline constexpr const char *CPX = "CPX";
// inline constexpr const char *CPY = "CPY";
// inline constexpr const char *DEC = "DEC";
// inline constexpr const char *DEX = "DEX";
// inline constexpr const char *DEY = "DEY";
// inline constexpr const char *EOR = "EOR";
// inline constexpr const char *INC = "INC";
// inline constexpr const char *INX = "INX";
// inline constexpr const char *INY = "INY";
// inline constexpr const char *JMP = "JMP";
// inline constexpr const char *JSR = "JSR";
// inline constexpr const char *LDA = "LDA";
// inline constexpr const char *LDX = "LDX";
// inline constexpr const char *LDY = "LDY";
// inline constexpr const char *LSR = "LSR";
// inline constexpr const char *NOP = "NOP";
// inline constexpr const char *ORA = "ORA";
// inline constexpr const char *PHA = "PHA";
// inline constexpr const char *PHP = "PHP";
// inline constexpr const char *PLA = "PLA";
// inline constexpr const char *PLP = "PLP";
// inline constexpr const char *ROL = "ROL";
// inline constexpr const char *ROR = "ROR";
// inline constexpr const char *RTI = "RTI";
// inline constexpr const char *RTS = "RTS";
// inline constexpr const char *SBC = "SBC";
// inline constexpr const char *SEC = "SEC";
// inline constexpr const char *SED = "SED";
// inline constexpr const char *SEI = "SEI";
// inline constexpr const char *STA = "STA";
// inline constexpr const char *STX = "STX";
// inline constexpr const char *STY = "STY";
// inline constexpr const char *TAX = "TAX";
// inline constexpr const char *TAY = "TAY";
// inline constexpr const char *TSX = "TSX";
// inline constexpr const char *TXA = "TXA";
// inline constexpr const char *TXS = "TXS";
// inline constexpr const char *TYA = "TYA";

enum {
    // == Non-Indexed, non memory ==

    // Accumulator
    // Some instructions have an option to operate directly upon the accumulator. The programmer specifies this by using
    // a special operand value, 'A'. For example:
    ACCUMULATOR, // 0, Num arg bytes: 1, num clock cycles: 2

    // Immediate
    // Immediate addressing allows the programmer to directly specify an 8 bit constant within the instruction. It is
    // indicated by a '#' symbol followed by an numeric expression. For example:
    IMMEDIATE, // 1, Num arg bytes: 2, num clock cycles: 2

    // Implicit
    // For many 6502 instructions the source and destination of the information to be manipulated is implied directly by
    // the function of the instruction itself and no further operand needs to be specified. Operations like 'Clear Carry
    // Flag' (CLC) and 'Return from Subroutine' (RTS) are implicit.
    IMPLIED, // 2, Num arg bytes: 2, num clock cycles: differs

    // == Non-Indexed memory ops ==

    // Relative
    // Relative addressing mode is used by branch instructions (e.g. BEQ, BNE, etc.) which contain a signed 8 bit
    // relative offset (e.g. -128 to +127) which is added to program counter if the condition is true. As the program
    // counter itself is incremented during instruction execution by two the effective address range for the target
    // instruction must be with -126 to +129 bytes of the branch.
    RELATIVE, // 3, Num arg bytes: 2, num clock cycles: 2

    // Absolute
    // Instructions using absolute addressing contain a full 16 bit address to identify the target location.
    ABSOLUTE, // 4, Num arg bytes: 3, num clock cycles: differs

    // Zero Page
    // An instruction using zero page addressing mode has only an 8 bit address operand. This limits it to addressing
    // only the first 256 bytes of memory (e.g. $0000 to $00FF) where the most significant byte of the address is always
    // zero. In zero page mode only the least significant byte of the address is held in the instruction making it
    // shorter by one byte (important for space saving) and one less memory fetch during execution (important for
    // speed).
    // An assembler will automatically select zero page addressing mode if the operand evaluates to a zero page address
    // and the instruction supports the mode (not all do).
    ZERO_PAGE, // 5, Num arg bytes: 2, num clock cycles: differs

    // Indirect
    // JMP is the only 6502 instruction to support indirection. The instruction contains a 16 bit address which
    // identifies the location of the least significant byte of another 16 bit memory address which is the real target
    // of the instruction.
    // For example if location $0120 contains $FC and location $0121 contains $BA then the instruction JMP ($0120) will
    // cause the next instruction execution to occur at $BAFC (e.g. the contents of $0120 and $0121).
    INDIRECT, // 6, Num arg bytes: 3, num clock cycles: 5

    // == Indexed memory ops ==

    // Absolute,X
    // The address to be accessed by an instruction using X register indexed absolute addressing is computed by taking
    // the 16 bit address from the instruction and added the contents of the X register. For example if X contains $92
    // then an STA $2000,X instruction will store the accumulator at $2092 (e.g. $2000 + $92).
    ABSOLUTE_X, // 7, Num arg bytes: 3, num clock cycles: differs

    // Absolute,Y
    // The Y register indexed absolute addressing mode is the same as the previous mode only with the contents of the Y
    // register added to the 16 bit address from the instruction.
    ABSOLUTE_Y, // 8, Num arg bytes: 3, num clock cycles: differs

    // Zero Page,X
    // The address to be accessed by an instruction using indexed zero page addressing is calculated by taking the 8 bit
    // zero page address from the instruction and adding the current value of the X register to it. For example if the X
    // register contains $0F and the instruction LDA $80,X is executed then the accumulator will be loaded from $008F
    // (e.g. $80 + $0F => $8F).
    // NB: The address calculation wraps around if the sum of the base address and the register exceed $FF. If we repeat
    // the last example but with $FF in the X register then the accumulator will be loaded from $007F (e.g. $80 + $FF =>
    // $7F) and not $017F.
    ZERO_PAGE_X, // 9, Num arg bytes: 2, num clock cycles: differs

    // Zero Page,Y
    // The address to be accessed by an instruction using indexed zero page addressing is calculated by taking the 8 bit
    // zero page address from the instruction and adding the current value of the Y register to it. This mode can only
    // be used with the LDX and STX instructions.
    ZERO_PAGE_Y, // 10, Num arg bytes: 2, num clock cycles: 4

    // Indexed Indirect
    // Indexed indirect addressing is normally used in conjunction with a table of address held on zero page. The
    // address of the table is taken from the instruction and the X register added to it (with zero page wrap around) to
    // give the location of the least significant byte of the target address.
    X_INDIRECT, // 11, Num arg bytes: 2, num clock cycles: 6

    // Indirect Indexed
    // Indirect indirect addressing is the most common indirection mode used on the 6502. In instruction contains the
    // zero page location of the least significant byte of 16 bit address. The Y register is dynamically added to this
    // value to generated the actual target address for operation.
    INDIRECT_Y, // 12, Num arg bytes: 2, num clock cycles: differs
};

inline std::string addrmode_to_string(uint8_t addrmode) {
    static const std::string list[] = {
        "acc", "imm", "impl", "rel", "abs", "zpage", "indi", "absx", "absy", "zpagex", "zpagey", "xindi", "indiy",
    };
    return list[addrmode];
}

struct Instruction {
    OpCode  mnemonic                      = OpCode::ADC;
    uint8_t opcode                        = 0;
    uint8_t addr_mode                     = IMPLIED;
    uint8_t num_args                      = 0;
    uint8_t cycles                        = 0;
    bool    extra_cycle_on_cross_boundary = false;

    static std::optional<Instruction> decode(uint8_t opcode);
    static const Instruction         *lut[256];
};

inline constexpr const Instruction instructions[] = {
    // ADC (ADd with Carry)
    // Affects Flags: N V Z C
    // ADC results are dependant on the setting of the decimal flag. In decimal mode, addition is carried out on the
    // assumption that the values involved are packed BCD (Binary Coded Decimal).
    // There is no way to add without carry.
    // Immediate     ADC #$44      $69  2   2
    {OpCode::ADC, 0x69, IMMEDIATE, 2, 2},
    // Zero Page     ADC $44       $65  2   3
    {OpCode::ADC, 0x65, ZERO_PAGE, 2, 3},
    // Zero Page,X   ADC $44,X     $75  2   4
    {OpCode::ADC, 0x75, ZERO_PAGE_X, 2, 4},
    // Absolute      ADC $4400     $6D  3   4
    {OpCode::ADC, 0x6D, ABSOLUTE, 3, 4},
    // Absolute,X    ADC $4400,X   $7D  3   4+
    {OpCode::ADC, 0x7D, ABSOLUTE_X, 3, 4, true},
    // Absolute,Y    ADC $4400,Y   $79  3   4+
    {OpCode::ADC, 0x79, ABSOLUTE_Y, 3, 4, true},
    // Indirect,X    ADC ($44,X)   $61  2   6
    {OpCode::ADC, 0x61, X_INDIRECT, 2, 6},
    // Indirect,Y    ADC ($44),Y   $71  2   5+
    {OpCode::ADC, 0x71, INDIRECT_Y, 2, 5, true},

    // AND (bitwise AND with accumulator)
    // Affects Flags: N Z
    // Immediate     AND #$44      $29  2   2
    {OpCode::AND, 0x29, IMMEDIATE, 2, 2},
    // Zero Page     AND $44       $25  2   3
    {OpCode::AND, 0x25, ZERO_PAGE, 2, 3},
    // Zero Page,X   AND $44,X     $35  2   4
    {OpCode::AND, 0x35, ZERO_PAGE_X, 2, 4},
    // Absolute      AND $4400     $2D  3   4
    {OpCode::AND, 0x2D, ABSOLUTE, 3, 4},
    // Absolute,X    AND $4400,X   $3D  3   4+
    {OpCode::AND, 0x3D, ABSOLUTE_X, 3, 4, true},
    // Absolute,Y    AND $4400,Y   $39  3   4+
    {OpCode::AND, 0x39, ABSOLUTE_Y, 3, 4, true},
    // Indirect,X    AND ($44,X)   $21  2   6
    {OpCode::AND, 0x21, X_INDIRECT, 2, 6},
    // Indirect,Y    AND ($44),Y   $31  2   5+
    {OpCode::AND, 0x31, INDIRECT_Y, 2, 5, true},

    // ASL (Arithmetic Shift Left)
    // Affects Flags: N Z C
    // ASL shifts all bits left one position. 0 is shifted into bit 0 and the original bit 7 is shifted into the Carry.
    // Accumulator   ASL A         $0A  1   2
    {OpCode::ASL, 0x0A, ACCUMULATOR, 1, 2},
    // Zero Page     ASL $44       $06  2   5
    {OpCode::ASL, 0x06, ZERO_PAGE, 2, 5},
    // Zero Page,X   ASL $44,X     $16  2   6
    {OpCode::ASL, 0x16, ZERO_PAGE_X, 2, 6},
    // Absolute      ASL $4400     $0E  3   6
    {OpCode::ASL, 0x0E, ABSOLUTE, 3, 6},
    // Absolute,X    ASL $4400,X   $1E  3   7
    {OpCode::ASL, 0x1E, ABSOLUTE_X, 3, 7},

    // BIT (test BITs)
    // Affects Flags: N V Z
    // BIT sets the Z flag as though the value in the address tested were ANDed with the accumulator. The N and V flags
    // are set to match bits 7 and 6 respectively in the value stored at the tested address.
    // BIT is often used to skip one or two following bytes as in:
    // CLOSE1 LDX #$10   If entered here, we
    //        .BYTE $2C  effectively perform
    // CLOSE2 LDX #$20   a BIT test on $20A2,
    //        .BYTE $2C  another one on $30A2,
    // CLOSE3 LDX #$30   and end up with the X
    // CLOSEX LDA #12    register still at $10
    //        STA ICCOM,X upon arrival here.
    // Beware: a BIT instruction used in this way as a NOP does have effects: the flags may be modified, and the read of
    // the absolute address, if it happens to access an I/O device, may cause an unwanted action.
    // Zero Page     BIT $44       $24  2   3
    {OpCode::BIT, 0x24, ZERO_PAGE, 2, 3},
    // Absolute      BIT $4400     $2C  3   4
    {OpCode::BIT, 0x2C, ABSOLUTE, 3, 4},

    // Branch Instructions
    // Affect Flags: none
    // All branches are relative mode and have a length of two bytes. Syntax is "Bxx Displacement" or (better) "Bxx
    // Label". See the notes on the Program Counter for more on displacements.
    // Branches are dependant on the status of the flag bits when the op code is encountered. A branch not taken
    // requires two machine cycles. Add one if the branch is taken and add one more if the branch crosses a page
    // boundary.
    // There is no BRA (BRanch Always) instruction but it can be easily emulated by branching on the basis of a known
    // condition. One of the best flags to use for this purpose is the oVerflow which is unchanged by all but addition
    // and subtraction operations.
    // A page boundary crossing occurs when the branch destination is on a different page than the instruction AFTER the
    // branch instruction. For example:
    //   SEC
    //   BCS LABEL
    //   NOP
    // A page boundary crossing occurs (i.e. the BCS takes 4 cycles) when (the address of) LABEL and the NOP are on
    // different pages. This means that
    //         CLV
    //         BVC LABEL
    //   LABEL NOP
    // the BVC instruction will take 3 cycles no matter what address it is located at.
    // BPL (Branch on PLus)           $10
    {OpCode::BPL, 0x10, RELATIVE, 2, 2},
    // BMI (Branch on MInus)          $30
    {OpCode::BMI, 0x30, RELATIVE, 2, 2},
    // BVC (Branch on oVerflow Clear) $50
    {OpCode::BVC, 0x50, RELATIVE, 2, 2},
    // BVS (Branch on oVerflow Set)   $70
    {OpCode::BVS, 0x70, RELATIVE, 2, 2},
    // BCC (Branch on Carry Clear)    $90
    {OpCode::BCC, 0x90, RELATIVE, 2, 2},
    // BCS (Branch on Carry Set)      $B0
    {OpCode::BCS, 0xB0, RELATIVE, 2, 2},
    // BNE (Branch on Not Equal)      $D0
    {OpCode::BNE, 0xD0, RELATIVE, 2, 2},
    // BEQ (Branch on EQual)          $F0
    {OpCode::BEQ, 0xF0, RELATIVE, 2, 2},

    // BRK (BReaK)
    // Affects Flags: B
    // BRK causes a non-maskable interrupt and increments the program counter by one. Therefore an RTI will go to the
    // address of the BRK +2 so that BRK may be used to replace a two-byte instruction for debugging and the subsequent
    // RTI will be correct.
    // Implied       BRK           $00  1   7
    {OpCode::BRK, 0x00, IMPLIED, 1, 7},

    // CMP (CoMPare accumulator)
    // Affects Flags: N Z C
    // Compare sets flags as if a subtraction had been carried out. If the value in the accumulator is equal or greater
    // than the compared value, the Carry will be set. The equal (Z) and negative (N) flags will be set based on
    // equality or lack thereof and the sign (i.e. A>=$80) of the accumulator.
    // Immediate     CMP #$44      $C9  2   2
    {OpCode::CMP, 0xC9, IMMEDIATE, 2, 2},
    // Zero Page     CMP $44       $C5  2   3
    {OpCode::CMP, 0xC5, ZERO_PAGE, 2, 3},
    // Zero Page,X   CMP $44,X     $D5  2   4
    {OpCode::CMP, 0xD5, ZERO_PAGE_X, 2, 4},
    // Absolute      CMP $4400     $CD  3   4
    {OpCode::CMP, 0xCD, ABSOLUTE, 3, 4},
    // Absolute,X    CMP $4400,X   $DD  3   4+
    {OpCode::CMP, 0xDD, ABSOLUTE_X, 3, 4, true},
    // Absolute,Y    CMP $4400,Y   $D9  3   4+
    {OpCode::CMP, 0xD9, ABSOLUTE_Y, 3, 4, true},
    // Indirect,X    CMP ($44,X)   $C1  2   6
    {OpCode::CMP, 0xC1, X_INDIRECT, 2, 6},
    // Indirect,Y    CMP ($44),Y   $D1  2   5+
    {OpCode::CMP, 0xD1, INDIRECT_Y, 2, 5, true},

    // CPX (ComPare X register)
    // Affects Flags: N Z C
    // Operation and flag results are identical to equivalent mode accumulator CMP ops.
    // Immediate     CPX #$44      $E0  2   2
    {OpCode::CPX, 0xE0, IMMEDIATE, 2, 2},
    // Zero Page     CPX $44       $E4  2   3
    {OpCode::CPX, 0xE4, ZERO_PAGE, 2, 3},
    // Absolute      CPX $4400     $EC  3   4
    {OpCode::CPX, 0xEC, ABSOLUTE, 3, 4},

    // CPY (ComPare Y register)
    // Affects Flags: N Z C
    // Operation and flag results are identical to equivalent mode accumulator CMP ops.
    // Immediate     CPY #$44      $C0  2   2
    {OpCode::CPY, 0xC0, IMMEDIATE, 2, 2},
    // Zero Page     CPY $44       $C4  2   3
    {OpCode::CPY, 0xC4, ZERO_PAGE, 2, 3},
    // Absolute      CPY $4400     $CC  3   4
    {OpCode::CPY, 0xCC, ABSOLUTE, 3, 4},

    // DEC (DECrement memory)
    // Affects Flags: N Z
    // Zero Page     DEC $44       $C6  2   5
    {OpCode::DEC, 0xC6, ZERO_PAGE, 2, 5},
    // Zero Page,X   DEC $44,X     $D6  2   6
    {OpCode::DEC, 0xD6, ZERO_PAGE_X, 2, 6},
    // Absolute      DEC $4400     $CE  3   6
    {OpCode::DEC, 0xCE, ABSOLUTE, 3, 6},
    // Absolute,X    DEC $4400,X   $DE  3   7
    {OpCode::DEC, 0xDE, ABSOLUTE_X, 3, 7},

    // EOR (bitwise Exclusive OR)
    // Affects Flags: N Z
    // Immediate     EOR #$44      $49  2   2
    {OpCode::EOR, 0x49, IMMEDIATE, 2, 2},
    // Zero Page     EOR $44       $45  2   3
    {OpCode::EOR, 0x45, ZERO_PAGE, 2, 3},
    // Zero Page,X   EOR $44,X     $55  2   4
    {OpCode::EOR, 0x55, ZERO_PAGE_X, 2, 4},
    // Absolute      EOR $4400     $4D  3   4
    {OpCode::EOR, 0x4D, ABSOLUTE, 3, 4},
    // Absolute,X    EOR $4400,X   $5D  3   4+
    {OpCode::EOR, 0x5D, ABSOLUTE_X, 3, 4, true},
    // Absolute,Y    EOR $4400,Y   $59  3   4+
    {OpCode::EOR, 0x59, ABSOLUTE_Y, 3, 4, true},
    // Indirect,X    EOR ($44,X)   $41  2   6
    {OpCode::EOR, 0x41, X_INDIRECT, 2, 6},
    // Indirect,Y    EOR ($44),Y   $51  2   5+
    {OpCode::EOR, 0x51, INDIRECT_Y, 2, 5, true},

    // Flag (Processor Status) Instructions
    // Affect Flags: as noted
    // These instructions are implied mode, have a length of one byte and require two machine cycles.
    // Notes:
    //   The Interrupt flag is used to prevent (SEI) or enable (CLI) maskable interrupts (aka IRQ's). It does not signal
    //   the presence or absence of an interrupt condition. The 6502 will set this flag automatically in response to an
    //   interrupt and restore it to its prior status on completion of the interrupt service routine. If you want your
    //   interrupt service routine to permit other maskable interrupts, you must clear the I flag in your code.
    //   The Decimal flag controls how the 6502 adds and subtracts. If set, arithmetic is carried out in packed binary
    //   coded decimal. This flag is unchanged by interrupts and is unknown on power-up. The implication is that a CLD
    //   should be included in boot or interrupt coding.
    //   The Overflow flag is generally misunderstood and therefore under-utilised. After an ADC or SBC instruction, the
    //   overflow flag will be set if the twos complement result is less than -128 or greater than +127, and it will
    //   cleared otherwise. In twos complement, $80 through $FF represents -128 through -1, and $00 through $7F
    //   represents 0 through +127. Thus, after:
    //   CLC
    //   LDA #$7F ;   +127
    //   ADC #$01 ; +   +1
    // the overflow flag is 1 (+127 + +1 = +128), and after:
    //   CLC
    //   LDA #$81 ;   -127
    //   ADC #$FF ; +   -1
    // the overflow flag is 0 (-127 + -1 = -128). The overflow flag is not affected by increments, decrements, shifts
    // and logical operations i.e. only ADC, BIT, CLV, PLP, RTI and SBC affect it. There is no op code to set the
    // overflow but a BIT test on an RTS instruction will do the trick.
    // CLC (CLear Carry)              $18
    {OpCode::CLC, 0x18, IMPLIED, 1, 2},
    // SEC (SEt Carry)                $38
    {OpCode::SEC, 0x38, IMPLIED, 1, 2},
    // CLI (CLear Interrupt)          $58
    {OpCode::CLI, 0x58, IMPLIED, 1, 2},
    // SEI (SEt Interrupt)            $78
    {OpCode::SEI, 0x78, IMPLIED, 1, 2},
    // CLV (CLear oVerflow)           $B8
    {OpCode::CLV, 0xB8, IMPLIED, 1, 2},
    // CLD (CLear Decimal)            $D8
    {OpCode::CLD, 0xD8, IMPLIED, 1, 2},
    // SED (SEt Decimal)              $F8
    {OpCode::SED, 0xF8, IMPLIED, 1, 2},

    // INC (INCrement memory)
    // Affects Flags: N Z
    // Zero Page     INC $44       $E6  2   5
    {OpCode::INC, 0xE6, ZERO_PAGE, 2, 5},
    // Zero Page,X   INC $44,X     $F6  2   6
    {OpCode::INC, 0xF6, ZERO_PAGE_X, 2, 6},
    // Absolute      INC $4400     $EE  3   6
    {OpCode::INC, 0xEE, ABSOLUTE, 3, 6},
    // Absolute,X    INC $4400,X   $FE  3   7
    {OpCode::INC, 0xFE, ABSOLUTE_X, 3, 7},

    // JMP (JuMP)
    // Affects Flags: none
    // JMP transfers program execution to the following address (absolute) or to the location contained in the following
    // address (indirect). Note that there is no carry associated with the indirect jump so:
    // AN INDIRECT JUMP MUST NEVER USE A
    // VECTOR BEGINNING ON THE LAST BYTE
    // OF A PAGE
    // For example if address $3000 contains $40, $30FF contains $80, and $3100 contains $50, the result of JMP ($30FF)
    // will be a transfer of control to $4080 rather than $5080 as you intended i.e. the 6502 took the low byte of the
    // address from $30FF and the high byte from $3000.
    // Absolute      JMP $5597     $4C  3   3
    {OpCode::JMP, 0x4C, ABSOLUTE, 3, 3},
    // Indirect      JMP ($5597)   $6C  3   5
    {OpCode::JMP, 0x6C, INDIRECT, 3, 5},

    // JSR (Jump to SubRoutine)
    // Affects Flags: none
    // JSR pushes the address-1 of the next operation on to the stack before transferring program control to the
    // following address. Subroutines are normally terminated by a RTS op code.
    // Absolute      JSR $5597     $20  3   6
    {OpCode::JSR, 0x20, ABSOLUTE, 3, 6},

    // LDA (LoaD Accumulator)
    // Affects Flags: N Z
    // Immediate     LDA #$44      $A9  2   2
    {OpCode::LDA, 0xA9, IMMEDIATE, 2, 2},
    // Zero Page     LDA $44       $A5  2   3
    {OpCode::LDA, 0xA5, ZERO_PAGE, 2, 3},
    // Zero Page,X   LDA $44,X     $B5  2   4
    {OpCode::LDA, 0xB5, ZERO_PAGE_X, 2, 4},
    // Absolute      LDA $4400     $AD  3   4
    {OpCode::LDA, 0xAD, ABSOLUTE, 3, 4},
    // Absolute,X    LDA $4400,X   $BD  3   4+
    {OpCode::LDA, 0xBD, ABSOLUTE_X, 3, 4, true},
    // Absolute,Y    LDA $4400,Y   $B9  3   4+
    {OpCode::LDA, 0xB9, ABSOLUTE_Y, 3, 4, true},
    // Indirect,X    LDA ($44,X)   $A1  2   6
    {OpCode::LDA, 0xA1, X_INDIRECT, 2, 6},
    // Indirect,Y    LDA ($44),Y   $B1  2   5+
    {OpCode::LDA, 0xB1, INDIRECT_Y, 2, 5},

    // LDX (LoaD X register)
    // Affects Flags: N Z
    // Immediate     LDX #$44      $A2  2   2
    {OpCode::LDX, 0xA2, IMMEDIATE, 2, 2},
    // Zero Page     LDX $44       $A6  2   3
    {OpCode::LDX, 0xA6, ZERO_PAGE, 2, 3},
    // Zero Page,Y   LDX $44,Y     $B6  2   4
    {OpCode::LDX, 0xB6, ZERO_PAGE_Y, 2, 4},
    // Absolute      LDX $4400     $AE  3   4
    {OpCode::LDX, 0xAE, ABSOLUTE, 3, 4},
    // Absolute,Y    LDX $4400,Y   $BE  3   4+
    {OpCode::LDX, 0xBE, ABSOLUTE_Y, 3, 4, true},

    // LDY (LoaD Y register)
    // Affects Flags: N Z
    // Immediate     LDY #$44      $A0  2   2
    {OpCode::LDY, 0xA0, IMMEDIATE, 2, 2},
    // Zero Page     LDY $44       $A4  2   3
    {OpCode::LDY, 0xA4, ZERO_PAGE, 2, 3},
    // Zero Page,X   LDY $44,X     $B4  2   4
    {OpCode::LDY, 0xB4, ZERO_PAGE_X, 2, 4},
    // Absolute      LDY $4400     $AC  3   4
    {OpCode::LDY, 0xAC, ABSOLUTE, 3, 4},
    // Absolute,X    LDY $4400,X   $BC  3   4+
    {OpCode::LDY, 0xBC, ABSOLUTE_X, 3, 4, true},

    // LSR (Logical Shift Right)
    // Affects Flags: N Z C
    // LSR shifts all bits right one position. 0 is shifted into bit 7 and the original bit 0 is shifted into the Carry.
    // Accumulator   LSR A         $4A  1   2
    {OpCode::LSR, 0x4A, ACCUMULATOR, 1, 2},
    // Zero Page     LSR $44       $46  2   5
    {OpCode::LSR, 0x46, ZERO_PAGE, 2, 5},
    // Zero Page,X   LSR $44,X     $56  2   6
    {OpCode::LSR, 0x56, ZERO_PAGE_X, 2, 6},
    // Absolute      LSR $4400     $4E  3   6
    {OpCode::LSR, 0x4E, ABSOLUTE, 3, 6},
    // Absolute,X    LSR $4400,X   $5E  3   7
    {OpCode::LSR, 0x5E, ABSOLUTE_X, 3, 7},

    // Wrap-Around
    // Use caution with indexed zero page operations as they are subject to wrap-around. For example, if the X register
    // holds $FF and you execute LDA $80,X you will not access $017F as you might expect; instead you access $7F i.e.
    // $80-1. This characteristic can be used to advantage but make sure your code is well commented.
    // It is possible, however, to access $017F when X = $FF by using the Absolute,X addressing mode of LDA $80,X. That
    // is, instead of:
    //   LDA $80,X    ; ZeroPage,X - the resulting object code is: B5 80
    // which accesses $007F when X=$FF, use:
    //   LDA $0080,X  ; Absolute,X - the resulting object code is: BD 80 00
    // which accesses $017F when X = $FF (a at cost of one additional byte and one additional cycle). All of the
    // ZeroPage,X and ZeroPage,Y instructions except STX ZeroPage,Y and STY ZeroPage,X have a corresponding Absolute,X
    // and Absolute,Y instruction. Unfortunately, a lot of 6502 assemblers don't have an easy way to force Absolute
    // addressing, i.e. most will assemble a LDA $0080,X as B5 80. One way to overcome this is to insert the bytes using
    // the .BYTE pseudo-op (on some 6502 assemblers this pseudo-op is called DB or DFB, consult the assembler
    // documentation) as follows:
    //   .BYTE $BD,$80,$00  ; LDA $0080,X (absolute,X addressing mode)
    // The comment is optional, but highly recommended for clarity.
    // In cases where you are writing code that will be relocated you must consider wrap-around when assigning dummy
    // values for addresses that will be adjusted. Both zero and the semi-standard $FFFF should be avoided for dummy
    // labels. The use of zero or zero page values will result in assembled code with zero page opcodes when you wanted
    // absolute codes. With $FFFF, the problem is in addresses+1 as you wrap around to page 0.

    // Program Counter
    // When the 6502 is ready for the next instruction it increments the program counter before fetching the
    // instruction. Once it has the op code, it increments the program counter by the length of the operand, if any.
    // This must be accounted for when calculating branches or when pushing bytes to create a false return address (i.e.
    // jump table addresses are made up of addresses-1 when it is intended to use an RTS rather than a JMP).
    // The program counter is loaded least signifigant byte first. Therefore the most signifigant byte must be pushed
    // first when creating a false return address.
    // When calculating branches a forward branch of 6 skips the following 6 bytes so, effectively the program counter
    // points to the address that is 8 bytes beyond the address of the branch opcode; and a backward branch of $FA
    // (256-6) goes to an address 4 bytes before the branch instruction.

    // Execution Times
    // Op code execution times are measured in machine cycles; one machine cycle equals one clock cycle. Many
    // instructions require one extra cycle for execution if a page boundary is crossed; these are indicated by a +
    // following the time values shown.

    // NOP (No OPeration)
    // Affects Flags: none
    // NOP is used to reserve space for future modifications or effectively REM out existing code.
    // Implied       NOP           $EA  1   2
    {OpCode::NOP, 0xEA, IMPLIED, 1, 2},

    // ORA (bitwise OR with Accumulator)
    // Affects Flags: N Z
    // Immediate     ORA #$44      $09  2   2
    {OpCode::ORA, 0x09, IMMEDIATE, 2, 2},
    // Zero Page     ORA $44       $05  2   3
    {OpCode::ORA, 0x05, ZERO_PAGE, 2, 3},
    // Zero Page,X   ORA $44,X     $15  2   4
    {OpCode::ORA, 0x15, ZERO_PAGE_X, 2, 4},
    // Absolute      ORA $4400     $0D  3   4
    {OpCode::ORA, 0x0D, ABSOLUTE, 3, 4},
    // Absolute,X    ORA $4400,X   $1D  3   4+
    {OpCode::ORA, 0x1D, ABSOLUTE_X, 3, 4, true},
    // Absolute,Y    ORA $4400,Y   $19  3   4+
    {OpCode::ORA, 0x19, ABSOLUTE_Y, 3, 4, true},
    // Indirect,X    ORA ($44,X)   $01  2   6
    {OpCode::ORA, 0x01, X_INDIRECT, 2, 6},
    // Indirect,Y    ORA ($44),Y   $11  2   5+
    {OpCode::ORA, 0x11, INDIRECT_Y, 2, 5, true},

    // Register Instructions
    // Affect Flags: N Z
    // These instructions are implied mode, have a length of one byte and require two machine cycles.
    // TAX (Transfer A to X)    $AA
    {OpCode::TAX, 0xAA, IMPLIED, 1, 2},
    // TXA (Transfer X to A)    $8A
    {OpCode::TXA, 0x8A, IMPLIED, 1, 2},
    // DEX (DEcrement X)        $CA
    {OpCode::DEX, 0xCA, IMPLIED, 1, 2},
    // INX (INcrement X)        $E8
    {OpCode::INX, 0xE8, IMPLIED, 1, 2},
    // TAY (Transfer A to Y)    $A8
    {OpCode::TAY, 0xA8, IMPLIED, 1, 2},
    // TYA (Transfer Y to A)    $98
    {OpCode::TYA, 0x98, IMPLIED, 1, 2},
    // DEY (DEcrement Y)        $88
    {OpCode::DEY, 0x88, IMPLIED, 1, 2},
    // INY (INcrement Y)        $C8
    {OpCode::INY, 0xC8, IMPLIED, 1, 2},

    // ROL (ROtate Left)
    // Affects Flags: N Z C
    // ROL shifts all bits left one position. The Carry is shifted into bit 0 and the original bit 7 is shifted into the
    // Carry.
    // Accumulator   ROL A         $2A  1   2
    {OpCode::ROL, 0x2A, ACCUMULATOR, 1, 2},
    // Zero Page     ROL $44       $26  2   5
    {OpCode::ROL, 0x26, ZERO_PAGE, 2, 5},
    // Zero Page,X   ROL $44,X     $36  2   6
    {OpCode::ROL, 0x36, ZERO_PAGE_X, 2, 6},
    // Absolute      ROL $4400     $2E  3   6
    {OpCode::ROL, 0x2E, ABSOLUTE, 3, 6},
    // Absolute,X    ROL $4400,X   $3E  3   7
    {OpCode::ROL, 0x3E, ABSOLUTE_X, 3, 7},

    // ROR (ROtate Right)
    // Affects Flags: N Z C
    // ROR shifts all bits right one position. The Carry is shifted into bit 7 and the original bit 0 is shifted into
    // the Carry.
    // Accumulator   ROR A         $6A  1   2
    {OpCode::ROR, 0x6A, ACCUMULATOR, 1, 2},
    // Zero Page     ROR $44       $66  2   5
    {OpCode::ROR, 0x66, ZERO_PAGE, 2, 5},
    // Zero Page,X   ROR $44,X     $76  2   6
    {OpCode::ROR, 0x76, ZERO_PAGE_X, 2, 6},
    // Absolute      ROR $4400     $6E  3   6
    {OpCode::ROR, 0x6E, ABSOLUTE, 3, 6},
    // Absolute,X    ROR $4400,X   $7E  3   7
    {OpCode::ROR, 0x7E, ABSOLUTE_X, 3, 7},

    // RTI (ReTurn from Interrupt)
    // Affects Flags: all
    // RTI retrieves the Processor Status Word (flags) and the Program Counter from the stack in that order (interrupts
    // push the PC first and then the PSW).
    // Note that unlike RTS, the return address on the stack is the actual address rather than the address-1.
    // Implied       RTI           $40  1   6
    {OpCode::RTI, 0x40, IMPLIED, 1, 6},

    // RTS (ReTurn from Subroutine)
    // Affects Flags: none
    // RTS pulls the top two bytes off the stack (low byte first) and transfers program control to that address+1. It is
    // used, as expected, to exit a subroutine invoked via JSR which pushed the address-1.
    // RTS is frequently used to implement a jump table where addresses-1 are pushed onto the stack and accessed via RTS
    // eg. to access the second of four routines:
    //  LDX #1
    //  JSR EXEC
    //  JMP SOMEWHERE
    // LOBYTE
    //  .BYTE <ROUTINE0-1,<ROUTINE1-1
    //  .BYTE <ROUTINE2-1,<ROUTINE3-1
    // HIBYTE
    //  .BYTE >ROUTINE0-1,>ROUTINE1-1
    //  .BYTE >ROUTINE2-1,>ROUTINE3-1
    // EXEC
    //  LDA HIBYTE,X
    //  PHA
    //  LDA LOBYTE,X
    //  PHA
    //  RTS
    // Implied       RTS           $60  1   6
    {OpCode::RTS, 0x60, IMPLIED, 1, 6},

    // SBC (SuBtract with Carry)
    // Affects Flags: N V Z C
    // SBC results are dependant on the setting of the decimal flag. In decimal mode, subtraction is carried out on the
    // assumption that the values involved are packed BCD (Binary Coded Decimal).
    // There is no way to subtract without the carry which works as an inverse borrow. i.e, to subtract you set the
    // carry before the operation. If the carry is cleared by the operation, it indicates a borrow occurred.
    // Immediate     SBC #$44      $E9  2   2
    {OpCode::SBC, 0xE9, IMMEDIATE, 2, 2},
    // Zero Page     SBC $44       $E5  2   3
    {OpCode::SBC, 0xE5, ZERO_PAGE, 2, 3},
    // Zero Page,X   SBC $44,X     $F5  2   4
    {OpCode::SBC, 0xF5, ZERO_PAGE_X, 2, 4},
    // Absolute      SBC $4400     $ED  3   4
    {OpCode::SBC, 0xED, ABSOLUTE, 3, 4},
    // Absolute,X    SBC $4400,X   $FD  3   4+
    {OpCode::SBC, 0xFD, ABSOLUTE_X, 3, 4, true},
    // Absolute,Y    SBC $4400,Y   $F9  3   4+
    {OpCode::SBC, 0xF9, ABSOLUTE_Y, 3, 4, true},
    // Indirect,X    SBC ($44,X)   $E1  2   6
    {OpCode::SBC, 0xE1, X_INDIRECT, 2, 6},
    // Indirect,Y    SBC ($44),Y   $F1  2   5+
    {OpCode::SBC, 0xF1, INDIRECT_Y, 2, 5, true},

    // STA (STore Accumulator)
    // Affects Flags: none
    // Zero Page     STA $44       $85  2   3
    {OpCode::STA, 0x85, ZERO_PAGE, 2, 3},
    // Zero Page,X   STA $44,X     $95  2   4
    {OpCode::STA, 0x95, ZERO_PAGE_X, 2, 4},
    // Absolute      STA $4400     $8D  3   4
    {OpCode::STA, 0x8D, ABSOLUTE, 3, 4},
    // Absolute,X    STA $4400,X   $9D  3   5
    {OpCode::STA, 0x9D, ABSOLUTE_X, 3, 5},
    // Absolute,Y    STA $4400,Y   $99  3   5
    {OpCode::STA, 0x99, ABSOLUTE_Y, 3, 5},
    // Indirect,X    STA ($44,X)   $81  2   6
    {OpCode::STA, 0x81, X_INDIRECT, 2, 6},
    // Indirect,Y    STA ($44),Y   $91  2   6
    {OpCode::STA, 0x91, INDIRECT_Y, 2, 6},

    // Stack Instructions
    // These instructions are implied mode, have a length of one byte and require machine cycles as indicated. The
    // "PuLl" operations are known as "POP" on most other microprocessors. With the 6502, the stack is always on page
    // one ($100-$1FF) and works top down.
    // TXS (Transfer X to Stack ptr)   $9A  2
    {OpCode::TXS, 0x9A, IMPLIED, 1, 2},
    // TSX (Transfer Stack ptr to X)   $BA  2
    {OpCode::TSX, 0xBA, IMPLIED, 1, 2},
    // PHA (PusH Accumulator)          $48  3
    {OpCode::PHA, 0x48, IMPLIED, 1, 3},
    // PLA (PuLl Accumulator)          $68  4
    {OpCode::PLA, 0x68, IMPLIED, 1, 4},
    // PHP (PusH Processor status)     $08  3
    {OpCode::PHP, 0x08, IMPLIED, 1, 3},
    // PLP (PuLl Processor status)     $28  4
    {OpCode::PLP, 0x28, IMPLIED, 1, 4},

    // STX (STore X register)
    // Affects Flags: none
    // Zero Page     STX $44       $86  2   3
    {OpCode::STX, 0x86, ZERO_PAGE, 2, 3},
    // Zero Page,Y   STX $44,Y     $96  2   4
    {OpCode::STX, 0x96, ZERO_PAGE_Y, 2, 4},
    // Absolute      STX $4400     $8E  3   4
    {OpCode::STX, 0x8E, ABSOLUTE, 3, 4},

    // STY (STore Y register)
    // Affects Flags: none
    // Zero Page     STY $44       $84  2   3
    {OpCode::STY, 0x84, ZERO_PAGE, 2, 3},
    // Zero Page,X   STY $44,X     $94  2   4
    {OpCode::STY, 0x94, ZERO_PAGE_X, 2, 4},
    // Absolute      STY $4400     $8C  3   4
    {OpCode::STY, 0x8C, ABSOLUTE, 3, 4},

};

struct Obj {
    typedef bool (Obj::*opcode_func)(cpu::Instruction &instr);
    typedef uint16_t (Obj::*addrmode_func)();

    enum class State {
        RESET,
        RUN,
        HALT,
    };

    BusHarness address_bus;
    BusHarness data_bus;
    BusHarness write_signal;

    cpumem::Registers registers;
    State             state               = State::RESET;
    size_t            subcycle_counter    = 0;
    size_t            actual_subcycle_max = 0;
    uint8_t           buffer[5];
    size_t            cycle_count;

    std::optional<Instruction> current_instruction = std::nullopt;
    // std::map<OpCode, opcode_func> opcode_funcs;
    std::vector<opcode_func>     opcode_funcs;
    std::map<int, addrmode_func> addrmode_funcs;

    Obj() : address_bus("cpu"), data_bus("cpu"), write_signal("cpu") {
        static bool has_init_opcodes = false;
        if (!has_init_opcodes) {
            memset(Instruction::lut, 0, sizeof(Instruction::lut));
            for (auto &instruction : instructions) {
                Instruction::lut[instruction.opcode] = &instruction;
            }
            has_init_opcodes = true;
        }
        opcode_funcs.reserve((size_t)OpCode::_NUM);
        opcode_funcs[(size_t)OpCode::ADC] = &Obj::_op_ADC;
        opcode_funcs[(size_t)OpCode::AND] = &Obj::_op_AND;
        opcode_funcs[(size_t)OpCode::ASL] = &Obj::_op_ASL;
        opcode_funcs[(size_t)OpCode::BCC] = &Obj::_op_BCC;
        opcode_funcs[(size_t)OpCode::BCS] = &Obj::_op_BCS;
        opcode_funcs[(size_t)OpCode::BEQ] = &Obj::_op_BEQ;
        opcode_funcs[(size_t)OpCode::BIT] = &Obj::_op_BIT;
        opcode_funcs[(size_t)OpCode::BMI] = &Obj::_op_BMI;
        opcode_funcs[(size_t)OpCode::BNE] = &Obj::_op_BNE;
        opcode_funcs[(size_t)OpCode::BPL] = &Obj::_op_BPL;
        opcode_funcs[(size_t)OpCode::BRK] = &Obj::_op_BRK;
        opcode_funcs[(size_t)OpCode::BVC] = &Obj::_op_BVC;
        opcode_funcs[(size_t)OpCode::BVS] = &Obj::_op_BVS;
        opcode_funcs[(size_t)OpCode::CLC] = &Obj::_op_CLC;
        opcode_funcs[(size_t)OpCode::CLD] = &Obj::_op_CLD;
        opcode_funcs[(size_t)OpCode::CLI] = &Obj::_op_CLI;
        opcode_funcs[(size_t)OpCode::CLV] = &Obj::_op_CLV;
        opcode_funcs[(size_t)OpCode::CMP] = &Obj::_op_CMP;
        opcode_funcs[(size_t)OpCode::CPX] = &Obj::_op_CPX;
        opcode_funcs[(size_t)OpCode::CPY] = &Obj::_op_CPY;
        opcode_funcs[(size_t)OpCode::DEC] = &Obj::_op_DEC;
        opcode_funcs[(size_t)OpCode::DEX] = &Obj::_op_DEX;
        opcode_funcs[(size_t)OpCode::DEY] = &Obj::_op_DEY;
        opcode_funcs[(size_t)OpCode::EOR] = &Obj::_op_EOR;
        opcode_funcs[(size_t)OpCode::INC] = &Obj::_op_INC;
        opcode_funcs[(size_t)OpCode::INX] = &Obj::_op_INX;
        opcode_funcs[(size_t)OpCode::INY] = &Obj::_op_INY;
        opcode_funcs[(size_t)OpCode::JMP] = &Obj::_op_JMP;
        opcode_funcs[(size_t)OpCode::JSR] = &Obj::_op_JSR;
        opcode_funcs[(size_t)OpCode::LDA] = &Obj::_op_LDA;
        opcode_funcs[(size_t)OpCode::LDX] = &Obj::_op_LDX;
        opcode_funcs[(size_t)OpCode::LDY] = &Obj::_op_LDY;
        opcode_funcs[(size_t)OpCode::LSR] = &Obj::_op_LSR;
        opcode_funcs[(size_t)OpCode::NOP] = &Obj::_op_NOP;
        opcode_funcs[(size_t)OpCode::ORA] = &Obj::_op_ORA;
        opcode_funcs[(size_t)OpCode::PHA] = &Obj::_op_PHA;
        opcode_funcs[(size_t)OpCode::PHP] = &Obj::_op_PHP;
        opcode_funcs[(size_t)OpCode::PLA] = &Obj::_op_PLA;
        opcode_funcs[(size_t)OpCode::PLP] = &Obj::_op_PLP;
        opcode_funcs[(size_t)OpCode::ROL] = &Obj::_op_ROL;
        opcode_funcs[(size_t)OpCode::ROR] = &Obj::_op_ROR;
        opcode_funcs[(size_t)OpCode::RTI] = &Obj::_op_RTI;
        opcode_funcs[(size_t)OpCode::RTS] = &Obj::_op_RTS;
        opcode_funcs[(size_t)OpCode::SBC] = &Obj::_op_SBC;
        opcode_funcs[(size_t)OpCode::SEC] = &Obj::_op_SEC;
        opcode_funcs[(size_t)OpCode::SED] = &Obj::_op_SED;
        opcode_funcs[(size_t)OpCode::SEI] = &Obj::_op_SEI;
        opcode_funcs[(size_t)OpCode::STA] = &Obj::_op_STA;
        opcode_funcs[(size_t)OpCode::STX] = &Obj::_op_STX;
        opcode_funcs[(size_t)OpCode::STY] = &Obj::_op_STY;
        opcode_funcs[(size_t)OpCode::TAX] = &Obj::_op_TAX;
        opcode_funcs[(size_t)OpCode::TAY] = &Obj::_op_TAY;
        opcode_funcs[(size_t)OpCode::TSX] = &Obj::_op_TSX;
        opcode_funcs[(size_t)OpCode::TXA] = &Obj::_op_TXA;
        opcode_funcs[(size_t)OpCode::TXS] = &Obj::_op_TXS;
        opcode_funcs[(size_t)OpCode::TYA] = &Obj::_op_TYA;

        addrmode_funcs[RELATIVE]  = &Obj::_addrmode_relative;
        addrmode_funcs[ABSOLUTE]  = &Obj::_addrmode_absolute;
        addrmode_funcs[ZERO_PAGE] = &Obj::_addrmode_zero_page;
        // addrmode_funcs[INDIRECT]    = &Obj::_addrmode_indirect;
        addrmode_funcs[ABSOLUTE_X]  = &Obj::_addrmode_absolute_x;
        addrmode_funcs[ABSOLUTE_Y]  = &Obj::_addrmode_absolute_y;
        addrmode_funcs[ZERO_PAGE_X] = &Obj::_addrmode_zero_page_x;
        addrmode_funcs[ZERO_PAGE_Y] = &Obj::_addrmode_zero_page_y;
        // addrmode_funcs[X_INDIRECT]  = &Obj::_addrmode_x_indirect;
        // addrmode_funcs[INDIRECT_Y]  = &Obj::_addrmode_indirect_y;
    }

    void reset() {
        state               = State::RESET;
        subcycle_counter    = 0;
        actual_subcycle_max = 0;
        memset(buffer, 0, sizeof(buffer));
        cycle_count = 0;
        current_instruction.reset();

        memset(&registers, 0, sizeof(registers));
        // registers.program_counter = 0xFFFC;
        registers.stack_pointer      = 0xFF;
        registers.processor_status.i = 1;
    }

    void cycle();

    uint8_t _read(uint16_t address);
    void    _write(uint8_t data, uint16_t address);

    void    _push(uint8_t data);
    uint8_t _pop();

    void _branch(bool do_branch);

    // uint16_t _addrmode_accumulator();
    // uint16_t _addrmode_immediate();
    // uint16_t _addrmode_implied();
    uint16_t _addrmode_relative();
    uint16_t _addrmode_absolute();
    uint16_t _addrmode_zero_page();
    uint16_t _addrmode_indirect1();
    uint16_t _addrmode_indirect2(uint16_t laddr);
    uint16_t _addrmode_absolute_x();
    uint16_t _addrmode_absolute_y();
    uint16_t _addrmode_zero_page_x();
    uint16_t _addrmode_zero_page_y();
    uint16_t _addrmode_x_indirect1();
    uint16_t _addrmode_x_indirect2(uint16_t laddr);
    uint16_t _addrmode_indirect_y1();
    uint16_t _addrmode_indirect_y2(uint16_t laddr);

    bool _op_ADC(Instruction &instr);
    bool _op_AND(Instruction &instr);
    bool _op_ASL(Instruction &instr);
    bool _op_BCC(Instruction &instr);
    bool _op_BCS(Instruction &instr);
    bool _op_BEQ(Instruction &instr);
    bool _op_BIT(Instruction &instr);
    bool _op_BMI(Instruction &instr);
    bool _op_BNE(Instruction &instr);
    bool _op_BPL(Instruction &instr);
    bool _op_BRK(Instruction &instr);
    bool _op_BVC(Instruction &instr);
    bool _op_BVS(Instruction &instr);
    bool _op_CLC(Instruction &instr);
    bool _op_CLD(Instruction &instr);
    bool _op_CLI(Instruction &instr);
    bool _op_CLV(Instruction &instr);
    bool _op_CMP(Instruction &instr);
    bool _op_CPX(Instruction &instr);
    bool _op_CPY(Instruction &instr);
    bool _op_DEC(Instruction &instr);
    bool _op_DEX(Instruction &instr);
    bool _op_DEY(Instruction &instr);
    bool _op_EOR(Instruction &instr);
    bool _op_INC(Instruction &instr);
    bool _op_INX(Instruction &instr);
    bool _op_INY(Instruction &instr);
    bool _op_JMP(Instruction &instr);
    bool _op_JSR(Instruction &instr);
    bool _op_LDA(Instruction &instr);
    bool _op_LDX(Instruction &instr);
    bool _op_LDY(Instruction &instr);
    bool _op_LSR(Instruction &instr);
    bool _op_NOP(Instruction &instr);
    bool _op_ORA(Instruction &instr);
    bool _op_PHA(Instruction &instr);
    bool _op_PHP(Instruction &instr);
    bool _op_PLA(Instruction &instr);
    bool _op_PLP(Instruction &instr);
    bool _op_ROL(Instruction &instr);
    bool _op_ROR(Instruction &instr);
    bool _op_RTI(Instruction &instr);
    bool _op_RTS(Instruction &instr);
    bool _op_SBC(Instruction &instr);
    bool _op_SEC(Instruction &instr);
    bool _op_SED(Instruction &instr);
    bool _op_SEI(Instruction &instr);
    bool _op_STA(Instruction &instr);
    bool _op_STX(Instruction &instr);
    bool _op_STY(Instruction &instr);
    bool _op_TAX(Instruction &instr);
    bool _op_TAY(Instruction &instr);
    bool _op_TSX(Instruction &instr);
    bool _op_TXA(Instruction &instr);
    bool _op_TXS(Instruction &instr);
    bool _op_TYA(Instruction &instr);
};

} // namespace cpu
