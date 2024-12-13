//
// Created by Bill Robinson on 12/07/2024.
//

#pragma once

#include <btBulletDynamicsCommon.h>
#include <Corrade/Containers/Pointer.h>
#include "MagnumGameCommon.h"

namespace MagnumGame {


    /**
     * @brief Scene Object with a Bullet rigid body attached
     */
    class RigidBody : public Object3D {
    public:

        enum class CollisionLayer : int {
            Terrain = 1,
            Dynamic = 2,
        };

        static constexpr int getLayerGroupMask(CollisionLayer layer) {
            return 1 << static_cast<int>(layer);
        }

        static constexpr int getLayerCollisionMask(CollisionLayer layer) {
            switch (layer) {
                case CollisionLayer::Terrain:
                    return ~getLayerGroupMask(CollisionLayer::Terrain);
                case CollisionLayer::Dynamic:
                default: return -1;
            }
        }

        RigidBody(Float mass, btCollisionShape *bShape, btDynamicsWorld &bWorld, CollisionLayer layer);

        ~RigidBody() override;

        btRigidBody &rigidBody() { return *_bRigidBody; }

        /* needed after changing the pose from Magnum side */
        void syncPose();

        btDynamicsWorld& getWorld() { return _bWorld; };

        void addToWorld();
        void removeFromWorld();


    private:
        btDynamicsWorld &_bWorld;
        Containers::Pointer<btRigidBody> _bRigidBody;
    };

}

