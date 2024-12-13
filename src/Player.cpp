//
// Created by Bill Robinson on 15/07/2024.
//

#include <vector>
#include "Player.h"

#include "DebugLines.h"
#include "RandomUtil.h"
#include "TexturedDrawable.h"
#include "RigidBody.h"

namespace MagnumGame {

    Player::Player(const std::string &name, RigidBody *pBody, IEnableDrawable *bodyDrawable)
        : _name(name), _pBody(pBody), _pBodyDrawable(bodyDrawable){
    }


    void Player::resetToStart(const Matrix4 &transformation) {
        // _pBody->rigidBody().setAngularVelocity({}); // Set angular velocity causes crashes... for some reason

        _pBody->rigidBody().setLinearVelocity({});
        // Debug{} << "resetting player" << getName().c_str() << transformation;
        _pBody->setTransformation(transformation);
        _pBody->syncPose();
    }

    bool Player::isAlive() const { return _pBody->rigidBody().isInWorld(); }

    void Player::update() {

    }

    Vector3 Player::getPosition() const { return _pBody->transformation().translation(); }

    void Player::die() {
        _pBody->removeFromWorld();
        auto transform = _pBody->transformation();
        transform.translation().y() = 1.0f/64;
        _pBody->setTransformation(transform);
    }

    void Player::revive() {
        _pBody->addToWorld();
        auto transform = _pBody->transformation();
        transform.translation().y() = 2.0f;
        _pBody->setTransformation(transform);
        _pBodyDrawable->setEnabled(true);
    }

    btDynamicsWorld & Player::getWorld() {
        return _pBody->getWorld();
    }
}
