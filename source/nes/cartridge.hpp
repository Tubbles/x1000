#pragma once

#include "mem.hpp"
#include <vector>

struct Cartridge {
    enum class Mirroring {
        VERTICAL,
        HORIZONTAL,
        FOUR_SCREEN,
    };

    std::vector<std::unique_ptr<uint8_t[]>> backends;
    std::vector<Memory>                     memories;

    uint8_t   mapper    = 0;
    Mirroring mirroring = Mirroring::VERTICAL;
};
