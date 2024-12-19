#pragma once

#include <SDL_events.h>

#include "MagnumGameCommon.h"
#include <Magnum/Math/Vector2.h>
#include <SDL2/SDL_gamecontroller.h>

#include "MagnumGameApp.h"

namespace MagnumGame {

    class GameState;


    class SdlGameController {
    public:
        explicit SdlGameController(Containers::Pointer<GameState>& gameState);
        ~SdlGameController();

        Vector2 readAxisVector(SDL_GameControllerAxis xAxis, SDL_GameControllerAxis yAxis) const;

        Vector2 getPlayerDirectionalControlVector() const;

        Vector2 getCameraDirectionalControlVector() const;

        bool handleEvent(const SDL_Event &event, const std::function<bool(MagnumGameApp::Key, MagnumGameApp::Modifiers)> &emulateKeyPress);

        static inline float JoystickDeadzone = 0.05f;

    private:
        SDL_GameController* _controller;
        Containers::Pointer<GameState>& _gameState;
    };

}



