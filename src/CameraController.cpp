#include "CameraController.h"

#include <Magnum/Math/Functions.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/Trade/CameraData.h>

namespace MagnumGame {
    CameraController::CameraController(Object3D &cameraObject, SceneGraph::Camera3D& camera)
        : _cameraObject(cameraObject), _camera(camera), _targetObject{}, _distance(10) {
    }

    void CameraController::setupTargetFromCurrent(Object3D &target) {
        _targetObject = &target;

        auto cameraFromTarget = _cameraObject.transformation().translation() - target.transformation().translation();
        _distance = cameraFromTarget.length();
        _yawAngle = Rad(atan2(cameraFromTarget.z(), cameraFromTarget.x()));
        // sin = opposite / hypotenuse
        _pitchAngle = Rad(asin(cameraFromTarget.y() / _distance));
    }

    void CameraController::rotateFromPointer(Vector2 eventPosDelta) {
        auto delta_yaw = 1.0_degf * eventPosDelta.x();
        auto delta_pitch = 1.0_degf * eventPosDelta.y();
        rotateBy(delta_yaw, delta_pitch);
    }

    void CameraController::loadCameraData(const Containers::Optional<Matrix4>& transformation,
        const Trade::CameraData& cameraData) {
        if (transformation) {
            _cameraObject.setTransformation(*transformation);
        }
        _camera.setProjectionMatrix(Matrix4::perspectiveProjection(
            cameraData.fov(),
            cameraData.aspectRatio(),
            cameraData.near(),
            cameraData.far()));
    }

    void CameraController::rotateBy(Deg deltaYaw, Deg deltaPitch) {
        if (_targetObject == nullptr) return;

        _yawAngle += Rad(deltaYaw);
        _pitchAngle += Rad(deltaPitch);
        _pitchAngle = clamp(_pitchAngle, Rad(10.0_degf), Rad(60.0_degf));
    }

    void CameraController::update([[maybe_unused]] Float deltaTime) {
        if (_targetObject == nullptr) return;

        auto cosYaw = cos(static_cast<float>(_yawAngle));
        auto sinYaw = sin(static_cast<float>(_yawAngle));
        auto cosPitch = cos(static_cast<float>(_pitchAngle));
        auto sinPitch = sin(static_cast<float>(_pitchAngle));

        Vector3 offset{
            cosYaw * cosPitch,
            sinPitch,
            sinYaw * cosPitch
        };

        offset *= _distance;

        auto targetPosition = _targetObject->transformation().translation();
        auto cameraPosition = targetPosition + offset;

        // If you want smooth camera movement, you can do something like this:
        // auto smoothHalfLife = 1.0f;
        // cameraPosition = lerp(_cameraObject.transformation().translation(), cameraPosition, smoothHalfLife * deltaTime);

        _cameraObject.setTransformation(Matrix4::lookAt(cameraPosition, targetPosition, {0,1,0}));
    }
}
