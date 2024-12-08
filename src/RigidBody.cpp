//
// Created by Bill Robinson on 12/07/2024.
//

#include "RigidBody.h"
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/BulletIntegration/MotionState.h>

namespace MagnumGame {
    RigidBody::RigidBody(Float mass, btCollisionShape *bShape, btDynamicsWorld &bWorld): Object3D{nullptr},
        _bWorld(bWorld) {
        /* Calculate inertia so the object reacts as it should with
               rotation and everything */
        btVector3 bInertia(0.0f, 0.0f, 0.0f);
        if (!Math::TypeTraits<Float>::equals(mass, 0.0f))
            bShape->calculateLocalInertia(mass, bInertia);

        //motionState is allocated here and added as a feature to this RigidBody, so deleted by it.
        auto motionState = new BulletIntegration::MotionState(*this);
        //It would be nice to do this: but it doesn't work because of addFeature being in AbstractObject and the MotionState template constructor requiring a transformation
        // auto& motionState = addFeature<BulletIntegration::MotionState>();
        _bRigidBody.emplace(btRigidBody::btRigidBodyConstructionInfo{
            mass, &motionState->btMotionState(), bShape, bInertia});
        _bRigidBody->forceActivationState(DISABLE_DEACTIVATION);
        bWorld.addRigidBody(_bRigidBody.get());
    }

    RigidBody::~RigidBody() {
        _bWorld.removeRigidBody(_bRigidBody.get());
    }

    void RigidBody::syncPose() {
        _bRigidBody->setWorldTransform(btTransform(transformationMatrix()));
    }

    void RigidBody::addToWorld() {
        _bWorld.addRigidBody(_bRigidBody.get());
    }

    void RigidBody::removeFromWorld() {
        _bWorld.removeRigidBody(_bRigidBody.get());
    }
}
