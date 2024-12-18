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
        explicit TexturedDrawable(Object3D &object,
                                  const Trade::ImageData2D &image,
                                  Shaders::PhongGL &shader,
                                  GL::Mesh& mesh,
                                  SceneGraph::DrawableGroup3D &drawables,
                                  UnsignedInt objectId = 0);
        explicit TexturedDrawable(Object3D &object,
                                  GL::Texture2D* texture,
                                  Shaders::PhongGL &shader,
                                  GL::Mesh& mesh,
                                  SceneGraph::DrawableGroup3D &drawables,
                                  UnsignedInt objectId = 0);
        explicit TexturedDrawable(Object3D &object,
                                  GL::Texture2D* texture,
                                  GameShader &shader,
                                  GL::Mesh& mesh,
                                  SceneGraph::DrawableGroup3D &drawables,
                                  UnsignedInt objectId = 0);

        bool isEnabled() const override;
        void setEnabled(bool b) override;
        void decayEnabled() override;

        btVector3 getPosition() const override;

        Object3D& getObject3D() const { return _object; }

        void setSkin(Skin& skin, UnsignedInt perVertexJointCount, UnsignedInt secondaryPerVertexJointCount);

        GL::Mesh& getMesh() const { return _mesh;}

        SkinMeshDrawable getSkinMeshDrawable() const { return _skinMeshDrawable; }

        static inline float ambientColor = 0.5f;
        static inline float lightColor = 0.5f;
        static inline float shininess = 10.0f;
        static inline float specular = 1.0f;
        static inline Vector3 lightDirection = {0.2f, 0.7f, 0.3f};

    private:
        void draw(const Matrix4 &transformation, SceneGraph::Camera3D &) override;
        static GL::Texture2D makeTexture(const Trade::ImageData2D &);

        Object3D& _object;
        Color4 _color;
        Containers::Optional<GL::Texture2D> _ownTexture;
        GL::Texture2D* _texture{};
        GL::Mesh& _mesh;
        Shaders::PhongGL* _phongShader;
        GameShader* _gameShader;

        UnsignedInt _objectId{};
        SkinMeshDrawable _skinMeshDrawable{};
    };

}
