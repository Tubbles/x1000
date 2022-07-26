#include "nes_main.hpp"

#include "nes/nes.hpp"
#include <random>

//

namespace nes {

std::random_device                                       random_device;
std::mt19937                                             random_number_generator(random_device());
std::uniform_int_distribution<std::mt19937::result_type> distribution(0x00, 0xFF);

static inline int get_random() {
    return distribution(random_number_generator);
}

NES nes;

void setup() {
    nes.get_random = get_random;
    nes.load_cartridge(binary_file_to_vector("/home/monkey/dev/x1000/test/nes/nestest/nestest.nes"));
}

void update() {
    // 1.7897725 MHz
    // 1789.7725 kHz
    // 1789772.5 Hz
    // Loop = 1/60 s = 60 Hz, 1789772.5 / 60 ~ 29830
    // constexpr const size_t nes_loops = 1;
    // constexpr const size_t nes_loops = 1250;
    constexpr const size_t nes_loops = 29830;
    for (size_t i = 0; i < nes_loops; i += 1) {
        // nes.ram_backend[SYS_RANDOM] = get_random();
        nes.main_cycle();
        // if (nes.cpu.state == cpu::Obj::State::HALT) {
        //     nes.reset();
        // }
    }
}

void keyboard_event(SDL_KeyboardEvent key) {
    if ((key.keysym.scancode == SDL_SCANCODE_R) && (key.keysym.mod & KMOD_CTRL)) {
        nes.reset();
    }
}

} // namespace nes
