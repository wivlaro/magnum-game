//
// Created by Bill Robinson on 15/07/2024.
//

#include <Magnum/BulletIntegration/Integration.h>
#include "Player.h"

#include "OnGroundQuery.h"
#include "TexturedDrawable.h"
#include "RigidBody.h"

namespace MagnumGame {



    Player::Player(const std::string &name, RigidBody *pBody, Animator *animator)
        : _name(name), _pBody(pBody), _animator(animator), _pBodyDrawable{} {
    }

    void Player::resetToStart(const Matrix4 &transformation) {
        // _pBody->rigidBody().setAngularVelocity({}); // Set angular velocity causes crashes... for some reason

        _pBody->rigidBody().setLinearVelocity({});
        // Debug{} << "resetting player" << getName().c_str() << transformation;
        _pBody->setTransformation(transformation);
        _pBody->syncPose();
    }

    bool Player::isAlive() const { return _pBody->rigidBody().isInWorld(); }

    void Player::update(Float deltaTime) {
        auto& rigidBody = _pBody->rigidBody();
        auto velocity = btVector3(_control * WalkSpeed);
        // auto motion = velocity * deltaTime;

        auto currentVelocity = rigidBody.getLinearVelocity();
        velocity.setY(currentVelocity.y());
        rigidBody.setLinearVelocity(btVector3(velocity));

        if (!_control.isZero()) {
            auto position = _pBody->transformation().translation();
            auto matrix = Matrix4::lookAt({0,0,0}, -_control, {0,1,0});
            matrix.translation() = position;
            _pBody->setTransformation(matrix);
            _pBody->syncPose();
        }

        _isOnGround = OnGroundQueryResult{rigidBody}.run(_pBody->getWorld());
        if (_markJumpFrame && _isOnGround) {
            rigidBody.applyImpulse({0,4.0f,0},{});
        }
        _markJumpFrame = false;

        if (_animator != nullptr) {
            if (!_isOnGround) {
                _animator->play("jump", false);
            }
            else if (_control.isZero()) {
                _animator->play("idle", false);
            }
            else {
                _animator->play("walk", false);
            }
        }

    }

    Vector3 Player::getPosition() const { return _pBody->transformation().translation(); }

    void Player::setControl(Vector2 controlVector, const Math::Matrix4<Float> &cameraObjectMatrix) {

        Vector3 cameraControlVector;
        if (!controlVector.isZero()) {
            cameraControlVector = cameraObjectMatrix.transformVector({ controlVector.x(), 0, -controlVector.y() });
            cameraControlVector.y() = 0;
            auto vectorLength = cameraControlVector.length();
            if (vectorLength > 1) {
                cameraControlVector /= vectorLength;
            }
        }
        else {
            cameraControlVector = {};
        }
        setControl(cameraControlVector);
    }

    void Player::setControl(const Vector3 &control) {
        if (_control != control) {
            _control = control;
        }
    }

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

    void Player::tryJump() {

        _markJumpFrame = true;
    }
}
