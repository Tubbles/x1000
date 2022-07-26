#pragma once

#include <SDL2/SDL.h>

namespace snake {
void setup();
void update();
void keyboard_event(SDL_KeyboardEvent key);
} // namespace snake
