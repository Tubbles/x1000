#include "gamepad.hpp"
#include "globals.hpp"
#include "nes_main.hpp"
#include "render.hpp"
#include "snake_main.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <argparse/argparse.hpp>
#include <chrono>
#include <fmt/core.h>
#include <functional>
#include <map>
#include <numeric>
#include <spdlog/spdlog.h>
#include <vector>

#include <SDL2/SDL_gamecontroller.h>

using namespace std::chrono_literals;

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
    // Point         target_frame_size = {32, 32};
    Point target_frame_size = {256, 240}; // NES

    // spdlog::set_level(spdlog::level::warn); // Set global log level
    spdlog::set_level(spdlog::level::info); // Set global log level
    // spdlog::set_level(spdlog::level::debug); // Set global log level
    // spdlog::set_level(spdlog::level::trace); // Set global log level

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    font              = TTF_OpenFont("/usr/share/fonts/TTF/Inconsolata-Regular.ttf", 14);
    auto num_displays = SDL_GetNumVideoDisplays();
    // SDL_WINDOWPOS_CENTERED_DISPLAY
    SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer);
    SDL_SetWindowFullscreen(window, 0);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED_DISPLAY(num_displays - 1),
                          SDL_WINDOWPOS_CENTERED_DISPLAY(num_displays - 1));
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    // SDL_RenderSetVSync(renderer, true);
    SDL_RenderSetVSync(renderer, false);
    SDL_GetWindowSize(window, &width, &height);

    pixel_size = std::min((int)std::floor(((float)width) / ((float)target_frame_size.x)),
                          (int)std::floor(((float)height) / ((float)target_frame_size.y)));
    // spdlog::debug("pixel_size = {}", pixel_size);

    auto cursor_surface = SDL_CreateRGBSurface(0, 8 * pixel_size, 8 * pixel_size, 32, 0, 0, 0, 0);
    memset(cursor_surface->pixels, 0xA0, 8 * pixel_size * 8 * pixel_size * 4);
    auto cursor = SDL_CreateColorCursor(cursor_surface, 0, 0);
    SDL_SetCursor(cursor);

    // SDL_ShowCursor(false);

    bool                    running     = true;
    int                     return_code = 0;
    std::vector<delta_time> frame_times;
    float                   fps = 0.0f;

    constexpr const uint32_t color_palette[] = {
        0x000000, 0xFFFFFF, 0x880000, 0xAAFFEE, 0xCC44CC, 0x00CC55, 0x0000AA, 0xEEEE77,
        0xDD8855, 0x664400, 0xFF7777, 0x333333, 0x777777, 0xAAFF66, 0x0088FF, 0xBBBBBB,
    };

    // static constexpr Point  origin{110, 10};
    // static constexpr int frame_length = 32;

    // static const Point origin{width / (2 * pixel_size) - (frame_length / 2),
    //                           height / (2 * pixel_size) - (frame_length / 2)};

    static const Point nes_origin{width / (2 * pixel_size) - (target_frame_size.x / 2),
                                  height / (2 * pixel_size) - (target_frame_size.y / 2)};

    auto previous_fps_timestamp = std::chrono::system_clock::now();
    auto total_time             = get_current_time();

    std::vector<Gamepad>                gamepads(16);
    std::map<SDL_JoystickID, Gamepad *> jids_to_gamepad;

    // nes::setup();
    snake::setup();

    RealPoint blip_coords{0.0, 0.0};

    while (running) {
        auto new_total_time = get_current_time();
        auto delta_time     = new_total_time - total_time;
        total_time          = new_total_time;
        frame_times.push_back(delta_time);

        auto current_fps_timestamp = std::chrono::system_clock::now();
        if (current_fps_timestamp - previous_fps_timestamp > 0.5s) {
            auto sum_frame_times =
                std::accumulate(frame_times.begin(), frame_times.end(), decltype(frame_times)::value_type(0));

            fps = 1.0f / (sum_frame_times.count() / frame_times.size());

            frame_times.clear();
            // spdlog::debug("FPS: {:.2F}", fps);
            // spdlog::debug("{}x{}", (width / pixel_size), (height / pixel_size));
            // spdlog::debug("Origin: {}x{}", origin.x, origin.y);

            previous_fps_timestamp = current_fps_timestamp;
        }

        frame_count += 1;

        int gamepad_active = -1;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                if ((event.key.keysym.scancode == SDL_SCANCODE_Q) && (event.key.keysym.mod & KMOD_CTRL)) {
                    running = false;
                }
                // if ((event.key.keysym.scancode == SDL_SCANCODE_W) || (event.key.keysym.scancode == SDL_SCANCODE_A)
                // ||
                //     (event.key.keysym.scancode == SDL_SCANCODE_S) || (event.key.keysym.scancode == SDL_SCANCODE_D))
                //     { nes.ram_backend[SYS_LASTKEY] = event.key.keysym.scancode;
                //     // spdlog::debug("Keypress: {:02X}", nes.ram_backend[SYS_LASTKEY]);
                // }
                // nes::keyboard_event(event.key);
                snake::keyboard_event(event.key);
                break;
            }
            case SDL_CONTROLLERBUTTONDOWN: {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE) {
                    running = false;
                }
                break;
            }
            case SDL_CONTROLLERDEVICEADDED: {
                SDL_ControllerDeviceEvent &cdevice = event.cdevice;
                spdlog::info("SDL_CONTROLLERDEVICEADDED, which: {}", cdevice.which);
                spdlog::info("Allocating gamepad {} from joystick", cdevice.which);
                auto &pad = gamepads[cdevice.which];
                pad.init(cdevice.which);
                spdlog::info("name: {}", SDL_GameControllerName(pad.controller));
                spdlog::info("haptics: {} trigger_haptics: {} accel: {} gyro: {}", pad.haptics_supported,
                             pad.trigger_haptics_supported, pad.accel_supported, pad.gyro_supported);
                jids_to_gamepad[SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad.controller))] = &pad;
                break;
            }
            case SDL_CONTROLLERDEVICEREMOVED: {
                SDL_ControllerDeviceEvent &cdevice = event.cdevice;
                auto                      &pad     = *jids_to_gamepad[cdevice.which];
                spdlog::info("SDL_CONTROLLERDEVICEREMOVED, which: {}", pad.index);
                spdlog::info("Deallocating gamepad {} from joystick", pad.index);
                pad.deinit();
                break;
            }
            case SDL_QUIT: {
                running = false;
                break;
            }

            case SDL_WINDOWEVENT:
                [[fallthrough]];
            case SDL_KEYUP:
                [[fallthrough]];
            case SDL_TEXTINPUT:
                [[fallthrough]];
            case SDL_MOUSEMOTION:
                [[fallthrough]];
            case SDL_MOUSEBUTTONDOWN:
                [[fallthrough]];
            case SDL_MOUSEBUTTONUP:
                [[fallthrough]];
            case SDL_JOYAXISMOTION:
                [[fallthrough]];
            case SDL_JOYBUTTONDOWN:
                [[fallthrough]];
            case SDL_JOYBUTTONUP:
                [[fallthrough]];
            case SDL_JOYDEVICEADDED:
                [[fallthrough]];
            case SDL_JOYDEVICEREMOVED:
                [[fallthrough]];
            case SDL_CONTROLLERAXISMOTION:
                [[fallthrough]];
            case SDL_CONTROLLERBUTTONUP:
                [[fallthrough]];
            case SDL_AUDIODEVICEADDED: {
                // TODO
                break;
            }
            default: {
                // spdlog::info("Unhandled SDL event: 0x{:X}", event.type);
                break;
            }
            }
        }

        int index = 0;
        for (auto &gamepad : gamepads) {
            gamepad.poll_state();
            if (gamepad.controller) {
                static const double speed = 0.05;
                blip_coords.x += (double)(gamepad.state.dpad_right - gamepad.state.dpad_left) * speed;
                blip_coords.y += (double)(gamepad.state.dpad_down - gamepad.state.dpad_up) * speed;
                blip_coords.x  = std::clamp(blip_coords.x, 0.0, (double)target_frame_size.x - 1);
                blip_coords.y  = std::clamp(blip_coords.y, 0.0, (double)target_frame_size.y - 1);
                gamepad_active = index;
            }

            if (gamepad.haptics_supported && ((int)blip_coords.x > target_frame_size.x)) {
                SDL_GameControllerRumble(gamepad.controller, 0xFFFF, 0xFFFF, 1000);
            }

            index += 1;
        }

        // nes::update();
        snake::update();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // Paint the large NES frame
        SDL_SetRenderDrawColor(renderer, cr(color_palette[15]), cg(color_palette[15]), cb(color_palette[15]), 255);
        for (int x = 0; x < target_frame_size.x + 2; x += 1) {
            render_pixel({x + nes_origin.x - 1, nes_origin.y - 1}, *renderer);
            render_pixel({x + nes_origin.x - 1, target_frame_size.y + nes_origin.y}, *renderer);
        }
        for (int y = 0; y < target_frame_size.y + 2; y += 1) {
            render_pixel({nes_origin.x - 1, y + nes_origin.y - 1}, *renderer);
            render_pixel({target_frame_size.x + nes_origin.x, y + nes_origin.y - 1}, *renderer);
        }

        // // Paint the small 32x32 frame
        // SDL_SetRenderDrawColor(renderer, cr(color_palette[15]), cg(color_palette[15]), cb(color_palette[15]), 255);
        // for (int x = 0; x < frame_length + 2; x += 1) {
        //     // clang-format off
        //     render_pixel({x + origin.x - 1,         origin.y - 1},              *renderer);
        //     render_pixel({x + origin.x - 1,         frame_length + origin.y},   *renderer);
        //     render_pixel({origin.x - 1,             x + origin.y - 1},          *renderer);
        //     render_pixel({frame_length + origin.x,  x + origin.y - 1},          *renderer);
        //     // clang-format on
        // }

        // // Paint the NES contents
        // for (size_t display_counter = 0x0200; display_counter < 0x0600; display_counter += 1) {
        //     auto color_index = nes.ram_backend[display_counter] % 16;
        //     SDL_SetRenderDrawColor(renderer, cr(color_palette[color_index]), cg(color_palette[color_index]),
        //                            cb(color_palette[color_index]), 255);
        //     render_pixel({(int)(display_counter % 32) + origin.x, (int)((display_counter - 0x0200) / 32) + origin.y},
        //                  *renderer);
        // }

        // Paint some random debug stuff
        // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        // render_pixel({(frame_count * 2) % (width / pixel_size), (frame_count) % (height / pixel_size)}, *renderer);
        // render_pixel({100, 100}, *renderer);

        // Paint FPS counter and resolution
        SDL_SetRenderDrawColor(renderer, cr(color_palette[15]), cg(color_palette[15]), cb(color_palette[15]), 255);
        int top = 0;
        render_text(fmt::format("FPS: {:.2F}", fps).c_str(), {0, top}, *renderer);
        top += 14;
        render_text(fmt::format("{}x{}", (width / pixel_size), (height / pixel_size)).c_str(), {0, top}, *renderer);
        top += 14;
        render_text(fmt::format("Pixelsize: {}", pixel_size).c_str(), {0, top}, *renderer);
        top += 14;
        render_text(fmt::format("GP: {}", gamepad_active).c_str(), {0, top}, *renderer);

        // Paint the blip
        SDL_SetRenderDrawColor(renderer, cr(color_palette[13]), cg(color_palette[13]), cb(color_palette[13]), 255);
        render_pixel({((int)blip_coords.x) + nes_origin.x, ((int)blip_coords.y) + nes_origin.y}, *renderer);

        SDL_RenderPresent(renderer);
    }

    // spdlog::set_level(spdlog::level::debug); // Set global log level
    // nes.print_mem(0x0200, 0x0400, 32);
    // nes.print_mem(SYS_RANDOM, 2);

    SDL_FreeCursor(cursor);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return return_code;
}
