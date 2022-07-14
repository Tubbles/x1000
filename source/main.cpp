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

#include <random>

std::random_device                                       random_device;
std::mt19937                                             random_number_generator(random_device());
std::uniform_int_distribution<std::mt19937::result_type> distribution(0x00, 0xFF);

enum {
    SYS_RANDOM  = 0x00FE,
    SYS_LASTKEY = 0x00FF,
};

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
    // spdlog::set_level(spdlog::level::trace); // Set global log level

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    font = TTF_OpenFont("/usr/share/fonts/TTF/Inconsolata-Regular.ttf", 14);
    SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
    // SDL_RenderSetVSync(renderer, true);
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

    NES                     nes;
    static constexpr Point  origin{110, 10};
    static constexpr size_t frame_length = 32;

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
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                if ((event.key.keysym.sym == SDLK_q) && (event.key.keysym.mod & KMOD_CTRL)) {
                    running = false;
                }
                if ((event.key.keysym.sym == SDLK_r) && (event.key.keysym.mod & KMOD_CTRL)) {
                    nes.reset();
                }
                if ((event.key.keysym.sym == SDLK_w) || (event.key.keysym.sym == SDLK_a) ||
                    (event.key.keysym.sym == SDLK_s) || (event.key.keysym.sym == SDLK_d)) {
                    nes.ram_backend[SYS_LASTKEY] = event.key.keysym.sym;
                    // spdlog::debug("Keypress: {:02X}", nes.ram_backend[SYS_LASTKEY]);
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
        }

        // 1.7897725 MHz
        // 1789.7725 kHz
        // 1789772.5 Hz
        // Loop = 1/60 s = 60 Hz, 1789772.5 / 60 = 29829.54
        for (size_t i = 0; i < 29830; i += 1) {
            nes.ram_backend[SYS_RANDOM] = distribution(random_number_generator);
            nes.cycle();
            if (nes.cpu.state == cpu::Obj::State::HALT) {
                nes.reset();
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // Paint the frame
        SDL_SetRenderDrawColor(renderer, cr(color_palette[15]), cg(color_palette[15]), cb(color_palette[15]), 255);
        for (size_t x = 0; x < frame_length + 2; x += 1) {
            // clang-format off
            render_pixel({(int)(x) + origin.x - 1,              (int)(0) + origin.y - 1},               *renderer);
            render_pixel({(int)(x) + origin.x - 1,              (int)(frame_length) + origin.y},        *renderer);
            render_pixel({(int)(0) + origin.x - 1,              (int)(x) + origin.y - 1},               *renderer);
            render_pixel({(int)(frame_length) + origin.x,       (int)(x) + origin.y - 1},               *renderer);
            // clang-format on
        }

        // Paint the NES contents
        for (size_t display_counter = 0x0200; display_counter < 0x0600; display_counter += 1) {
            auto color_index = nes.ram_backend[display_counter] % 16;
            SDL_SetRenderDrawColor(renderer, cr(color_palette[color_index]), cg(color_palette[color_index]),
                                   cb(color_palette[color_index]), 255);
            render_pixel({(int)(display_counter % 32) + origin.x, (int)((display_counter - 0x0200) / 32) + origin.y},
                         *renderer);
        }

        // Paint some random debug stuff
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        render_pixel({(frame_count * 2) % (width / pixel_size), (frame_count) % (height / pixel_size)}, *renderer);
        render_pixel({100, 100}, *renderer);

        // Paint FPS counter and resolution
        render_text(fmt::format("FPS: {:.2F}", fps).c_str(), {0, 0}, *renderer);
        render_text(fmt::format("{}x{}", (width / pixel_size), (height / pixel_size)).c_str(), {0, 14}, *renderer);

        SDL_RenderPresent(renderer);
    }

    spdlog::set_level(spdlog::level::debug); // Set global log level
    nes.print_mem(0x0200, 0x0400, 32);
    nes.print_mem(SYS_RANDOM, 2);

    SDL_FreeCursor(cursor);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return return_code;
}
