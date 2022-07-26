#pragma once

#include <SDL2/SDL.h>

namespace nes {
void setup();
void update();
void keyboard_event(SDL_KeyboardEvent key);
} // namespace nes
