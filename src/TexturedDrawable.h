//
// Created by Bill Robinson on 15/07/2024.
//

#pragma once

#include <Corrade/Containers/Optional.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Trade/Trade.h>
#include <Magnum/GL/GL.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Shaders/Shaders.h>

#include "Animator.h"
#include "IEnableDrawable.h"


namespace MagnumGame {
    class GameShader;

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
                                  GL::Texture2D* texture,
                                  Shaders::PhongGL &shader,
                                  GL::Mesh& mesh,
                                  SceneGraph::DrawableGroup3D &drawables,
                                  UnsignedInt objectId = 0);
        explicit TexturedDrawable(SceneGraph::AbstractObject3D &object,
                                  GL::Texture2D* texture,
                                  GameShader &shader,
                                  GL::Mesh& mesh,
                                  SceneGraph::DrawableGroup3D &drawables,
                                  UnsignedInt objectId = 0);

        bool isEnabled() const override;
        void setEnabled(bool b) override;
        void decayEnabled() override;

        btVector3 getPosition() const override;

        void setSkin(Skin& skin, UnsignedInt perVertexJointCount, UnsignedInt secondaryPerVertexJointCount);

        static inline float ambientColour = 0.2f;
        static inline float lightColour = 1.0f;
        static inline float shininess = 20.0f;
        static inline float specular = 0.3f;
        static inline Vector3 lightDirection = {0.4f, 0.5f, 0.3f};

    private:
        void draw(const Matrix4 &transformation, SceneGraph::Camera3D &) override;
        static GL::Texture2D makeTexture(const Trade::ImageData2D &);

        Color4 _color;
        Containers::Optional<GL::Texture2D> _ownTexture;
        GL::Texture2D* _texture{};
        GL::Mesh& _mesh;
        Shaders::PhongGL* _phongShader;
        GameShader* _gameShader;

        UnsignedInt _objectId{};
        Containers::Array<Matrix4>* _boneMatrices{};
        UnsignedInt _perVertexJointCount{};
        UnsignedInt _secondaryPerVertexJointCount{};
    };

}
