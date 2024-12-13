#pragma once

#include "MagnumGameCommon.h"
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/Trade/CameraData.h>

namespace MagnumGame {
    class CameraController {
    public:
        explicit CameraController(Object3D &cameraObject, SceneGraph::Camera3D &camera);

        void setupTargetFromCurrent(Object3D &target);

        void rotateBy(Deg deltaYaw, Deg deltaPitch);

        void update(Float deltaTime);

        Matrix4 getTransformationProjectionMatrix() const {
            return _camera.projectionMatrix() * _camera.cameraMatrix();
        }

        void draw(SceneGraph::DrawableGroup3D& drawableGroup) const { _camera.draw(drawableGroup); }

        void rotateFromPointer(Vector2 delta);

        Math::Matrix4<Float> getCameraObjectMatrix() const { return _cameraObject.absoluteTransformationMatrix(); }

        void loadCameraData(const Containers::Optional<Matrix4>& transformation, const Trade::CameraData& camera_data);

    private:
        Object3D &_cameraObject;
        SceneGraph::Camera3D &_camera;
        Object3D *_targetObject;

        float _distance;
        Rad _yawAngle;
        Rad _pitchAngle;
    };
}
