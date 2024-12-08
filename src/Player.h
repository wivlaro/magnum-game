//
// Created by Bill Robinson on 15/07/2024.
//

#pragma once

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include "TexturedDrawable.h"

namespace MagnumGame {
    class UnlitAlphaDrawable;
    class IEnableDrawable;
    class RigidBody;

    /**
     * @brief
     */
    class Player {

    public:

        Player(const std::string &name, RigidBody *pBody, IEnableDrawable* bodyDrawable);

        void update();

        Vector3 getPosition() const;

        void resetToStart(const Matrix4 &transformation);

        bool isAlive() const;

        btVector3 hitPosition() const;

        void die();
        void revive();

        const std::string &getName() const { return _name; }

        btDynamicsWorld& getWorld();

    private:
        std::string _name;
        RigidBody *_pBody;
        IEnableDrawable *_pBodyDrawable;
    };

}
