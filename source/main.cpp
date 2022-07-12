#include "globals.hpp"
#include "nes/nes.hpp"
#include "render.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <fmt/core.h>
#include <numeric>
#include <spdlog/spdlog.h>
#include <vector>

auto get_current_time = std::chrono::steady_clock::now;
using delta_time      = std::chrono::duration<float>;

constexpr uint8_t cr(uint32_t color_32bit) {
    return (color_32bit & 0x00FF0000) >> 16;
}

constexpr uint8_t cg(uint32_t color_32bit) {
    return (color_32bit & 0x0000FF00) >> 8;
}

constexpr uint8_t cb(uint32_t color_32bit) {
    return color_32bit & 0x000000FF;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    SDL_Event     event;
    SDL_Renderer *renderer = NULL;
    SDL_Window   *window   = NULL;
    int           width, height, frame_count = 0;

    spdlog::set_level(spdlog::level::debug); // Set global log level

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    font = TTF_OpenFont("/usr/share/fonts/TTF/Inconsolata-Regular.ttf", 14);
    SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
    SDL_RenderSetVSync(renderer, true);
    SDL_GetWindowSize(window, &width, &height);

    auto cursor_surface = SDL_CreateRGBSurface(0, 8 * pixel_size, 8 * pixel_size, 32, 0, 0, 0, 0);
    memset(cursor_surface->pixels, 0xA0, 8 * pixel_size * 8 * pixel_size * 4);
    auto cursor = SDL_CreateColorCursor(cursor_surface, 0, 0);
    SDL_SetCursor(cursor);

    // SDL_ShowCursor(false);

    bool                    running     = true;
    int                     return_code = 0;
    std::vector<delta_time> frame_times;
    float                   fps = 0.0f;

    auto total_time = get_current_time();

    constexpr const uint32_t color_palette[] = {
        0x000000, 0xFFFFFF, 0x880000, 0xAAFFEE, 0xCC44CC, 0x00CC55, 0x0000AA, 0xEEEE77,
        0xDD8855, 0x664400, 0xFF7777, 0x333333, 0x777777, 0xAAFF66, 0x0088FF, 0xBBBBBB,
    };

    NES nes;
    nes.cycle();
    nes.cycle();
    nes.cycle();
    nes.cycle();
    nes.cycle();
    nes.cycle();
    nes.cycle();
    nes.cycle();

    while (running) {
        auto new_total_time = get_current_time();
        auto delta_time     = new_total_time - total_time;
        total_time          = new_total_time;

        frame_times.push_back(delta_time);
        constexpr int num_frames_window = 60;
        if (frame_times.size() == num_frames_window) {
            fps =
                1.0f / (std::accumulate(frame_times.begin(), frame_times.end(), decltype(frame_times)::value_type(0)) /
                        num_frames_window)
                           .count();
            frame_times.clear();
        }

        frame_count += 1;
        SDL_PollEvent(&event);
        switch (event.type) {
        case SDL_KEYDOWN: {
            if ((event.key.keysym.sym == SDLK_q) && (event.key.keysym.mod & KMOD_CTRL)) {
                running = false;
            }
            break;
        }
        case SDL_QUIT: {
            running = false;
            break;
        }
        default: {
            break;
        }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        for (size_t stack_counter = 0x01FF; stack_counter >= 0x0100; stack_counter -= 1) {
            auto color_index = nes.ram_backend[stack_counter];
            SDL_SetRenderDrawColor(renderer, cr(color_palette[color_index]), cg(color_palette[color_index]),
                                   cb(color_palette[color_index]), 255);
            render_pixel({(int)(stack_counter % 16), (int)(stack_counter / 16)}, *renderer);
        }
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        render_pixel({(frame_count * 2) % (width / pixel_size), (frame_count) % (height / pixel_size)}, *renderer);
        render_pixel({100, 100}, *renderer);
        render_text(fmt::format("FPS: {:.2F}", fps).c_str(), {0, 0}, *renderer);
        render_text(fmt::format("{}x{}", (width / pixel_size), (height / pixel_size)).c_str(), {0, 14}, *renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_FreeCursor(cursor);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return return_code;
}
