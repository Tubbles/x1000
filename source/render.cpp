#include "render.hpp"
#include "globals.hpp"

void render_pixel(Point point, SDL_Renderer &renderer) {
    for (auto x = 0; x < pixel_size; x += 1) {
        for (auto y = 0; y < pixel_size; y += 1) {
            SDL_RenderDrawPoint(&renderer, (point.x * pixel_size) + x, (point.y * pixel_size) + y);
        }
    }
}

void render_text(const char *text, Point point, SDL_Renderer &renderer) {
    auto *surface = TTF_RenderText_Solid(font, text, {255, 255, 255, 0});
    auto *texture = SDL_CreateTextureFromSurface(&renderer, surface);
    SDL_Rect rect = {point.x * pixel_size, point.y * pixel_size, surface->w * pixel_size, surface->h * pixel_size};
    SDL_RenderCopy(&renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
