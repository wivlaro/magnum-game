#include "TrackingCamera.h"

namespace MagnumGame {
    TrackingCamera::TrackingCamera(Object3D &cameraObject)
        : _cameraObject(cameraObject) {
    }

    void TrackingCamera::setupTargetFromCurrent(Object3D &target) {
        _targetObject = &target;

        auto cameraFromTarget = _cameraObject.transformation().translation() - target.transformation().translation();
        _distance = cameraFromTarget.length();
        _yawAngle = Rad(atan2(cameraFromTarget.z(), cameraFromTarget.x()));
        // sin = opposite / hypotenuse
        _pitchAngle = Rad(asin(cameraFromTarget.y() / _distance));
    }

    void TrackingCamera::rotateBy(Deg deltaYaw, Deg deltaPitch) {
        if (_targetObject == nullptr) return;

        _yawAngle += Rad(deltaYaw);
        _pitchAngle += Rad(deltaPitch);
        _pitchAngle = clamp(_pitchAngle, Rad(10.0_degf), Rad(60.0_degf));
    }

    void TrackingCamera::update(Float deltaTime) {
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
        _cameraObject.setTransformation(Matrix4::lookAt(cameraPosition, targetPosition, {0,1,0}));
    }
}
