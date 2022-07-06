#pragma once

#include "util.hpp"
#include <cstddef>
#include <cstdint>

namespace cpumem {
struct Map {
    uint8_t ram[0x2000 - 0x0000];
    uint8_t ioreg[0x4020 - 0x2000];
    uint8_t cartmem[0x6000 - 0x4020];
    uint8_t saveram[0x8000 - 0x6000];
    uint8_t prgrom[0xFFFF - 0x8000];
};

enum {
    RAM_START = offsetof(Map, ram),
    RAM_SIZE = ARR_LEN(Map::ram),
    IOREG_START = offsetof(Map, ioreg),
    IOREG_SIZE = ARR_LEN(Map::ioreg),
    CARTMEM_START = offsetof(Map, cartmem),
    CARTMEM_SIZE = ARR_LEN(Map::cartmem),
    SAVERAM_START = offsetof(Map, saveram),
    SAVERAM_SIZE = ARR_LEN(Map::saveram),
    PRGROM_START = offsetof(Map, prgrom),
    PRGROM_SIZE = ARR_LEN(Map::prgrom),
};

struct ProcessorStatus {
    uint8_t n : 1;
    uint8_t v : 1;
    uint8_t _ : 1;
    uint8_t b : 1;
    uint8_t d : 1;
    uint8_t i : 1;
    uint8_t z : 1;
    uint8_t c : 1;
};

extern Map map;

extern uint16_t program_counter;
extern uint8_t stack_pointer;
extern uint8_t accumulator;
extern uint8_t index_register_x;
extern uint8_t index_register_y;
extern ProcessorStatus processor_status;

} // namespace cpumem

// namespace ppumem {
// struct Map {
//     uint8_t ram[0x2000 - 0x0000];
//     uint8_t ioreg[0x4020 - 0x2000];
//     uint8_t cartmem[0x6000 - 0x4020];
//     uint8_t saveram[0x8000 - 0x6000];
//     uint8_t prgrom[0xFFFF - 0x8000];
// };

// enum {
//     RAM_START = offsetof(Map, ram),
//     RAM_SIZE = ARR_LEN(Map::ram),
//     IOREG_START = offsetof(Map, ioreg),
//     IOREG_SIZE = ARR_LEN(Map::ioreg),
//     CARTMEM_START = offsetof(Map, cartmem),
//     CARTMEM_SIZE = ARR_LEN(Map::cartmem),
//     SAVERAM_START = offsetof(Map, saveram),
//     SAVERAM_SIZE = ARR_LEN(Map::saveram),
//     PRGROM_START = offsetof(Map, prgrom),
//     PRGROM_SIZE = ARR_LEN(Map::prgrom),
// };

// extern Map map;
// } // namespace ppumem
