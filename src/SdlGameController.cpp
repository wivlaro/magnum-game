//
// Created by Bill Robinson on 19/12/2024.
//

#include "SdlGameController.h"
#include "GameState.h"
#include "Player.h"

namespace MagnumGame {
    static SDL_GameController *findController() {
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (SDL_IsGameController(i)) {
                return SDL_GameControllerOpen(i);
            }
        }

        return nullptr;
    }

    SdlGameController::SdlGameController(Containers::Pointer<GameState> &gameState)
        : _controller(findController()),
          _gameState(gameState) {
    }

    SdlGameController::~SdlGameController() {
        if (_controller) {
            SDL_GameControllerClose(_controller);
        }
    }

    Vector2 SdlGameController::readAxisVector(::SDL_GameControllerAxis xAxis, ::SDL_GameControllerAxis yAxis) const {
        Vector2 input{
            static_cast<float>(SDL_GameControllerGetAxis(_controller, xAxis)) / (1 << 15),
            static_cast<float>(SDL_GameControllerGetAxis(_controller, yAxis)) / (1 << 15)
        };

        if (abs(input.x()) < JoystickDeadzone) {
            input.x() = 0;
        }
        if (abs(input.y()) < JoystickDeadzone) {
            input.y() = 0;
        }
        return input;
    }

    Vector2 SdlGameController::getPlayerDirectionalControlVector() const {
        if (_controller) {
            auto vector = readAxisVector(SDL_CONTROLLER_AXIS_LEFTX, SDL_CONTROLLER_AXIS_LEFTY);
            vector.y() = -vector.y();
            return vector;
        }
        return {0, 0};
    }

    Vector2 SdlGameController::getCameraDirectionalControlVector() const {
        if (_controller) {
            return readAxisVector(SDL_CONTROLLER_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTY);
        }
        return {0, 0};
    }


    bool SdlGameController::handleEvent(const SDL_Event &event, const std::function<bool(MagnumGameApp::Key, MagnumGameApp::Modifiers)> &emulateKeyPress) {
        switch (event.type) {
            case SDL_CONTROLLERDEVICEADDED:
                if (!_controller) {
                    _controller = SDL_GameControllerOpen(event.cdevice.which);
                    return true;
                }
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                if (_controller && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(_controller))) {
                    SDL_GameControllerClose(_controller);
                    _controller = findController();
                    return true;
                }
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                switch (event.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        return emulateKeyPress(MagnumGameApp::Key::Up, MagnumGameApp::Modifiers{});
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        return emulateKeyPress(MagnumGameApp::Key::Down, MagnumGameApp::Modifiers{});
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                        return emulateKeyPress(MagnumGameApp::Key::Left, MagnumGameApp::Modifiers{});
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                        return emulateKeyPress(MagnumGameApp::Key::Right, MagnumGameApp::Modifiers{});
                    case SDL_CONTROLLER_BUTTON_A:
                        return emulateKeyPress(MagnumGameApp::Key::Space, MagnumGameApp::Modifiers{});
                    case SDL_CONTROLLER_BUTTON_START:
                        return emulateKeyPress(MagnumGameApp::Key::Esc, MagnumGameApp::Modifiers{});
                }
        }
        return false;
    }
}
