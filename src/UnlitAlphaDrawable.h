//
// Created by Bill Robinson on 17/07/2024.
//

#pragma once
#include "MagnumGameCommon.h"
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/GL/GL.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Shaders/Shaders.h>

#include "IEnableDrawable.h"

namespace MagnumGame {
    class UnlitAlphaShader;

    using namespace Magnum;


/**
 * @brief Unlit, alpha-blended drawable Magnum Feature, attached to a scene object.
 */
class UnlitAlphaDrawable : public SceneGraph::Drawable3D, public IEnableDrawable {

public:
    UnlitAlphaDrawable(SceneGraph::AbstractObject3D &object, SceneGraph::DrawableGroup3D& drawables, const std::shared_ptr<GL::Texture2D> &texture, GL::Mesh &mesh, Shaders::FlatGL3D &shader);

    void setEnabled(bool shouldEnable) override;

    void decayEnabled() override;

    bool isEnabled() const override;

    btVector3 getPosition() const override;

private:
    void draw(const Matrix4 &transformation, SceneGraph::Camera3D &) override;


    Color4 _color;
    std::shared_ptr<GL::Texture2D> _texture;
    GL::Mesh& _mesh;
    Shaders::FlatGL3D& _shader;
};

}
