#pragma once
#include <vector>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/Math/Vector3.h>

#include "MagnumGameApp.h"

namespace MagnumGame {
    class Player;

    class GameState {
    public:
        GameState();

        void start();
        void update();

    private:
        std::unique_ptr<Player> _player;
        bool _isStarted = false;

        btSphereShape _bSphereQueryShape{0.4f};
        btGhostObject _bSphereQueryObject;


    };

}
