#pragma once

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>

struct OnGroundQueryResult : btCollisionWorld::ContactResultCallback {
    btRigidBody& rigidBody;

    bool onGround = false;

    OnGroundQueryResult(btRigidBody &rigid_body)
        : rigidBody(rigid_body) {
    }

    btScalar addSingleResult(btManifoldPoint& cp,
                             const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
                             const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override {
        if (colObj0Wrap && colObj1Wrap) {
            if (colObj0Wrap->m_collisionObject == &rigidBody) {
                if (cp.m_localPointA.y() < 0.0f && colObj1Wrap->m_collisionObject->isStaticObject()) {
                    onGround = true;
                }
            }
            else if (colObj1Wrap->m_collisionObject == &rigidBody) {
                if (cp.m_localPointB.y() < 0.0f && colObj0Wrap->m_collisionObject->isStaticObject()) {
                    onGround = true;
                }
            }
        }
        return 0;
    }

    bool run(btCollisionWorld& world) {
        world.contactTest(&rigidBody, *this);
        return onGround;
    }

};