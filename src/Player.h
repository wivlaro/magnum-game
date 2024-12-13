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

        inline static Float WalkSpeed = 4.0f;

        Player(const std::string &name, RigidBody *pBody, Animator* animator);

        void update(Float deltaTime);

        Vector3 getPosition() const;

        void resetToStart(const Matrix4 &transformation);

        bool isAlive() const;

        btVector3 hitPosition() const;

        void setControl(Vector2 controlVector, const Math::Matrix4<Float>& cameraObjectMatrix);
        void setControl(const Vector3& control);

        void die();
        void revive();

        const std::string &getName() const { return _name; }

        btDynamicsWorld& getWorld();

        void tryJump();

        RigidBody* getBody() { return _pBody; }

    private:
        std::string _name;
        RigidBody *_pBody;
        Animator *_animator;
        IEnableDrawable *_pBodyDrawable{};
        Vector3 _control{};
    };

}
