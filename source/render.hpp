#pragma once

#include "point.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void render_pixel(Point point, SDL_Renderer &renderer);
void render_text(const char *text, Point point, SDL_Renderer &renderer);
