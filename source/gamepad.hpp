#pragma once

// Shamelessly adapted and slightly more opinionated from isaboll1, thanks!
// https://github.com/isaboll1/SDL2-Gamepad-Example

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <cassert>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

struct Gamepad {
    int                 index      = -1;
    SDL_GameController *controller = nullptr;

    struct State {
        // Axis values range from -1.0f to 1.0f
        struct LeftStickAxis {
            float x = 0.0f;
            float y = 0.0f;
        } left_stick;

        // Axis values range from -1.0f - 1.0f
        struct RightStickAxis {
            float x = 0.0f;
            float y = 0.0f;
        } right_stick;

        int a_button = 0;
        int b_button = 0;
        int x_button = 0;
        int y_button = 0;

        int dpad_up    = 0;
        int dpad_down  = 0;
        int dpad_left  = 0;
        int dpad_right = 0;

        int left_shoulder     = 0;
        int right_shoulder    = 0;
        int left_stick_click  = 0;
        int right_stick_click = 0;

        int start = 0;
        int back  = 0;

        int touchpad = 0;
        int guide    = 0;
        int misc     = 0;

        int paddle1 = 0;
        int paddle2 = 0;
        int paddle3 = 0;
        int paddle4 = 0;

        // Axis values range from 0.0f to 1.0f
        float left_trigger = 0.0f;
        // Axis values range from 0.0f to 1.0f
        float right_trigger = 0.0f;
    } state, last_state;

    struct SensorState {
        // Explaination taken from SDL_sensor.h
        // For game controllers held in front of you,
        // the axes are defined as follows:
        // -X ... +X : left ... right
        // -Y ... +Y : bottom ... top
        // -Z ... +Z : farther ... closer

        // values[0]: Acceleration on the x axis
        // values[1]: Acceleration on the y axis
        // values[2]: Acceleration on the z axis
        float accelerometer[3] = {0.0f, 0.0f, 0.0f};

        // values[0]: Angular speed around the x axis (pitch)
        // values[1]: Angular speed around the y axis (yaw)
        // values[2]: Angular speed around the z axis (roll)
        float gyroscope[3] = {0.0f, 0.0f, 0.0f};
    } sensor_state, last_sensor_state;

    struct TouchpadFinger {
        uint8_t state;
        float   x        = 0.0f;
        float   y        = 0.0f;
        float   pressure = 0.0f;
    };

    struct Touchpad {
        std::vector<TouchpadFinger> fingers;
    };

    std::vector<Touchpad> touchpads;
    int                   touchpad_count = 0;

    bool haptics_supported         = false;
    bool trigger_haptics_supported = false;
    bool accel_supported           = false;
    bool gyro_supported            = false;

    struct VibrationValues {
        float motor_left    = 0.0;
        float motor_right   = 0.0;
        float trigger_left  = 0.0;
        float trigger_right = 0.0;
    } vibration;

    void init(int index) {
        auto &self = *this;

        if (self.controller) {
            return;
        }

        self.index      = index;
        self.controller = SDL_GameControllerOpen(self.index);
        if (!self.controller) {
            SPDLOG_ERROR("{}:{}: {}", __FILE__, __LINE__, SDL_GetError());
        }

        // Check motors
        self.haptics_supported         = (SDL_GameControllerRumble(self.controller, 0, 0, 0) == 0);
        self.trigger_haptics_supported = (SDL_GameControllerRumbleTriggers(self.controller, 0, 0, 0) == 0);

        // Check sensors
        self.accel_supported = SDL_GameControllerHasSensor(self.controller, SDL_SENSOR_ACCEL);
        self.gyro_supported  = SDL_GameControllerHasSensor(self.controller, SDL_SENSOR_GYRO);

        // Check touchpads
        self.touchpad_count = SDL_GameControllerGetNumTouchpads(self.controller);
        if (self.touchpad_count) {
            self.touchpads.resize(self.touchpad_count);
            for (int i = 0; i < self.touchpad_count; i++) {
                self.touchpads[i].fingers.resize(SDL_GameControllerGetNumTouchpadFingers(self.controller, i));
            }
        }
    }

    void deinit() {
        auto &self = *this;

        if (!self.controller) {
            return;
        }

        SDL_GameControllerClose(self.controller);
        self.index      = -1;
        self.controller = nullptr;
    }

    void poll_state() {
        auto &self = *this;

        if (!self.controller) {
            return;
        }

        self.last_state = self.state;
        self.state      = State();

        self.last_sensor_state = self.sensor_state;
        self.sensor_state      = SensorState();

        // DPad buttons
        self.state.dpad_up    = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
        self.state.dpad_down  = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        self.state.dpad_left  = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        self.state.dpad_right = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

        // Face Buttons (based on Xbox controller layout)
        self.state.a_button = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_A);
        self.state.b_button = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_B);
        self.state.x_button = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_X);
        self.state.y_button = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_Y);

        // Start, Back, and Guide
        self.state.start = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_START);
        self.state.back  = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_BACK);
        self.state.guide = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_GUIDE);

        // Left Click and Right Click
        self.state.left_stick_click  = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
        self.state.right_stick_click = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);

        // Paddles 1-4
        self.state.paddle1 = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_PADDLE1);
        self.state.paddle2 = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_PADDLE2);
        self.state.paddle3 = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_PADDLE3);
        self.state.paddle4 = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_PADDLE4);

        // Touchpad Button and Misc (Xbox Share button, Switch Pro Capture button, and Mic button for PS4/PS5
        // controllers)
        self.state.touchpad = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_TOUCHPAD);
        self.state.misc     = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_MISC1);

        // Left and Right Shoulder
        self.state.left_shoulder  = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        self.state.right_shoulder = SDL_GameControllerGetButton(self.controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

        // Axis values for the left and right stick
        self.state.left_stick.x =
            float(SDL_GameControllerGetAxis(self.controller, SDL_CONTROLLER_AXIS_LEFTX)) / float(SDL_JOYSTICK_AXIS_MAX);
        self.state.left_stick.y =
            float(SDL_GameControllerGetAxis(self.controller, SDL_CONTROLLER_AXIS_LEFTY)) / float(SDL_JOYSTICK_AXIS_MAX);
        self.state.right_stick.x = float(SDL_GameControllerGetAxis(self.controller, SDL_CONTROLLER_AXIS_RIGHTX)) /
                                   float(SDL_JOYSTICK_AXIS_MAX);
        self.state.right_stick.y = float(SDL_GameControllerGetAxis(self.controller, SDL_CONTROLLER_AXIS_RIGHTY)) /
                                   float(SDL_JOYSTICK_AXIS_MAX);

        // Left and Right Trigger
        self.state.left_trigger = float(SDL_GameControllerGetAxis(self.controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) /
                                  float(SDL_JOYSTICK_AXIS_MAX);
        self.state.right_trigger = float(SDL_GameControllerGetAxis(self.controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) /
                                   float(SDL_JOYSTICK_AXIS_MAX);

        // Sensors
        if (accel_supported) {
            SDL_GameControllerGetSensorData(self.controller, SDL_SENSOR_ACCEL, self.sensor_state.accelerometer, 3);
        }
        if (gyro_supported) {
            SDL_GameControllerGetSensorData(self.controller, SDL_SENSOR_GYRO, self.sensor_state.gyroscope, 3);
        }

        // Touchpads
        for (int index = 0; index < self.touchpad_count; index++) {
            for (size_t finger = 0; finger < self.touchpads[index].fingers.size(); finger++) {
                // clang-format off
                SDL_GameControllerGetTouchpadFinger(
                    self.controller, index, finger,
                    &self.touchpads[index].fingers[finger].state,
                    &self.touchpads[index].fingers[finger].x,
                    &self.touchpads[index].fingers[finger].y,
                    &self.touchpads[index].fingers[finger].pressure
                );
                // clang-format on
            }
        }
    }
};
