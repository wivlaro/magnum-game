//
// Created by Bill Robinson on 15/07/2024.
//

#include <Magnum/GL/Sampler.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include "TexturedDrawable.h"


namespace MagnumGame {
    TexturedDrawable::TexturedDrawable(SceneGraph::AbstractObject3D &object,
                                       const Trade::ImageData2D &image,
                                       Magnum::Shaders::PhongGL &shader,
                                       Magnum::GL::Mesh &mesh,
                                       SceneGraph::DrawableGroup3D &drawables,
                                       UnsignedInt objectId)
        : SceneGraph::Drawable3D(object, &drawables),
          _color{0xffffffff_rgbaf},
          _ownTexture{makeTexture(image)},
          _texture(&*_ownTexture),
          _mesh{mesh},
          _shader{shader},
          _objectId{objectId} {
    }

    TexturedDrawable::TexturedDrawable(SceneGraph::AbstractObject3D &object,
                                       GL::Texture2D *image,
                                       Magnum::Shaders::PhongGL &shader,
                                       Magnum::GL::Mesh &mesh,
                                       SceneGraph::DrawableGroup3D &drawables,
                                       UnsignedInt objectId)
        : SceneGraph::Drawable3D{object, &drawables},
          _color{0xffffffff_rgbaf},
          _ownTexture{Containers::NullOpt},
          _texture(image),
          _mesh{mesh},
          _shader{shader},
          _objectId(objectId) {
    }

    GL::Texture2D &&TexturedDrawable::makeTexture(const Trade::ImageData2D &image) {
        GL::Texture2D t{};
        t.setWrapping(GL::SamplerWrapping::ClampToEdge)
                .setMagnificationFilter(GL::SamplerFilter::Linear)
                .setMinificationFilter(GL::SamplerFilter::Linear)
                .setStorage(1, GL::textureFormat(image.format()), image.size())
                .setSubImage(0, {}, image);
        return std::move(t);
    }

    void TexturedDrawable::draw(const Magnum::Matrix4 &transformation, SceneGraph::Camera3D &camera) {
        if (_color.a() <= 0.0f) return;
        CHECK_GL_ERROR(__FILE__, __LINE__);
        _shader.setProjectionMatrix(camera.projectionMatrix());
        _shader.setTransformationMatrix(transformation);
        _shader.setDiffuseColor(_color);
        if (_texture) {
            _shader.bindDiffuseTexture(*_texture);
            { GL::Renderer::Error err; while ((err = GL::Renderer::error()) != GL::Renderer::Error::NoError) { Error() << ("TexturedDrawable.cpp" ":" "67") << "Error: " << err << _texture << _texture->id(); } };
        }
        if (_shader.flags() & Shaders::PhongGL::Flag::ObjectId) {
            _shader.setObjectId(_objectId);
        }
        if (_shader.flags() & Shaders::PhongGL::Flag::DynamicPerVertexJointCount) {
            if (_boneMatrices != nullptr) {
                _shader.setPerVertexJointCount(_perVertexJointCount, _secondaryPerVertexJointCount);
                CHECK_GL_ERROR(__FILE__, __LINE__);
                _shader.setJointMatrices(*_boneMatrices);
                CHECK_GL_ERROR(__FILE__, __LINE__);
            } else {
                _shader.setPerVertexJointCount(0, 0);
            }
        }
        CHECK_GL_ERROR(__FILE__, __LINE__);

        _shader.draw(_mesh);
    }

    void TexturedDrawable::setEnabled(bool b) {
        if (b) {
            _color = 0xffffffff_rgbaf;
        } else {
            _color.a() = 0.0f;
        }
    }

    void TexturedDrawable::decayEnabled() {
        _color *= 0.9f;
    }

    bool TexturedDrawable::isEnabled() const {
        return _color.a() >= 1.0f;
    }

    btVector3 TexturedDrawable::getPosition() const {
        auto translation = object().absoluteTransformationMatrix().translation();
        return btVector3{
            translation.x(),
            translation.y(),
            translation.z()
        };
    }

    void TexturedDrawable::setSkin(Skin &skin, UnsignedInt perVertexJointCount,
                                   UnsignedInt secondaryPerVertexJointCount) {
        _boneMatrices = &skin.boneMatrices();
        _perVertexJointCount = perVertexJointCount;
        _secondaryPerVertexJointCount = secondaryPerVertexJointCount;
    }
}
