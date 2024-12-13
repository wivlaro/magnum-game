//
// Created by Bill Robinson on 17/07/2024.
//

#include "UnlitAlphaDrawable.h"
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Shaders/FlatGL.h>

namespace MagnumGame {
    UnlitAlphaDrawable::UnlitAlphaDrawable(SceneGraph::AbstractObject3D &object, SceneGraph::DrawableGroup3D &drawables,
                                           const std::shared_ptr<GL::Texture2D> &texture, GL::Mesh &mesh, Shaders::FlatGL3D &shader): SceneGraph::Drawable3D(object, &drawables),
        _color(0xffffffff_rgbaf),
        _texture(texture),
        _mesh(mesh),
        _shader(shader) {
    }

    void UnlitAlphaDrawable::draw(const Matrix4 &transformation, SceneGraph::Camera3D &camera) {
        _shader.setTransformationProjectionMatrix(camera.projectionMatrix() * transformation);
        _shader.bindTexture(*_texture)
            .setColor(_color)
            .draw(_mesh);
    }

    void UnlitAlphaDrawable::setEnabled(bool shouldEnable) {
        _color.a() = shouldEnable ? 1.0f : 0.0f;
    }
    void UnlitAlphaDrawable::decayEnabled() {
        _color.a() *= 0.9f;
    }

    bool UnlitAlphaDrawable::isEnabled() const {
        return _color.a() >= 1.0f;
    }

    btVector3 UnlitAlphaDrawable::getPosition() const {
        auto position = object().absoluteTransformationMatrix().translation();
        return btVector3{position.x(), position.y(), position.z()};
    }
}


