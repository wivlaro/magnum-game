#include "GameState.h"

#include <sstream>
#include <Corrade/Utility/DebugStl.h>
#include "Player.h"

namespace MagnumGame {
    GameState::GameState(Timeline& timeline) : _timeline(timeline) {
        _bSphereQueryObject.setCollisionShape(&_bSphereQueryShape);

    }

    void GameState::setupPlayer(RigidBody *rigidBody, Animator *animator) {
        _player = std::make_unique<Player>("Someone", rigidBody, animator);
    }

    void GameState::start() {
        _isStarted = true;
    }

    void GameState::update() {
        _player->update(_timeline.previousFrameDuration());
    }
}
