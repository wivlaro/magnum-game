#pragma once

#include "MagnumGameCommon.h"
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Object.h>

namespace MagnumGame {
    class TrackingCamera {
    public:
        explicit TrackingCamera(Object3D &camera);

        void setupTargetFromCurrent(Object3D &target);

        void rotateBy(Deg deltaYaw, Deg deltaPitch);

        void update(Float deltaTime);

    private:
        Object3D &_cameraObject;
        Object3D *_targetObject;

        float _distance;
        Rad _yawAngle;
        Rad _pitchAngle;
    };
}
