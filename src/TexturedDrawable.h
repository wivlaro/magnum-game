//
// Created by Bill Robinson on 15/07/2024.
//

#pragma once

#include <Corrade/Containers/Optional.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Trade/Trade.h>
#include <Magnum/GL/GL.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Shaders/Shaders.h>

#include "Animator.h"
#include "IEnableDrawable.h"
#include "MagnumGameCommon.h"


namespace MagnumGame {

    /**
     * @brief Textured, opaque drawable Magnum Feature, attached to a scene object.
     *
     * It can also be enabled/disabled to show/hide it
     */
    class TexturedDrawable : public SceneGraph::Drawable3D, public IEnableDrawable {

    public:
        explicit TexturedDrawable(SceneGraph::AbstractObject3D &object,
                                  const Trade::ImageData2D &image,
                                  Shaders::PhongGL &shader,
                                  GL::Mesh& mesh,
                                  SceneGraph::DrawableGroup3D &drawables,
                                  UnsignedInt objectId = 0);
        explicit TexturedDrawable(SceneGraph::AbstractObject3D &object,
                                  GL::Texture2D* image,
                                  Shaders::PhongGL &shader,
                                  GL::Mesh& mesh,
                                  SceneGraph::DrawableGroup3D &drawables,
                                  UnsignedInt objectId = 0);

        bool isEnabled() const override;
        void setEnabled(bool b) override;
        void decayEnabled() override;

        btVector3 getPosition() const override;


        void setSkin(Skin& skin, UnsignedInt perVertexJointCount, UnsignedInt secondaryPerVertexJointCount);

    private:
        void draw(const Matrix4 &transformation, SceneGraph::Camera3D &) override;
        static GL::Texture2D&& makeTexture(const Trade::ImageData2D&);

        Color4 _color;
        Containers::Optional<GL::Texture2D> _ownTexture;
        GL::Texture2D* _texture;
        GL::Mesh& _mesh;
        Shaders::PhongGL& _shader;

        UnsignedInt _objectId;
        Containers::Array<Matrix4>* _boneMatrices;
        UnsignedInt _perVertexJointCount;
        UnsignedInt _secondaryPerVertexJointCount;
    };

} // NamePicker
