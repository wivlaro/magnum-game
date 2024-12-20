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

#include "GameShader.h"


namespace MagnumGame {
    TexturedDrawable::TexturedDrawable(Object3D &object,
                                       const Trade::ImageData2D &image,
                                       Shaders::PhongGL &shader,
                                       GL::Mesh &mesh,
                                       SceneGraph::DrawableGroup3D &drawables,
                                       UnsignedInt objectId)
        : SceneGraph::Drawable3D(object, &drawables),
            _object(object),
          _color{0xffffffff_rgbaf},
          _ownTexture{makeTexture(image)},
          _texture(&*_ownTexture),
          _mesh{mesh},
          _phongShader{&shader},
          _gameShader{nullptr},
          _objectId{objectId} {
    }

    TexturedDrawable::TexturedDrawable(Object3D &object,
                                       GL::Texture2D *texture,
                                       Shaders::PhongGL &shader,
                                       GL::Mesh &mesh,
                                       SceneGraph::DrawableGroup3D &drawables,
                                       UnsignedInt objectId)
        : SceneGraph::Drawable3D{object, &drawables},
            _object(object),
          _color{0xffffffff_rgbaf},
          _ownTexture{Containers::NullOpt},
          _texture(texture),
          _mesh{mesh},
          _phongShader{&shader},
          _gameShader{nullptr},
          _objectId(objectId) {
        if (_texture) {
            Debug{} << "TexturedDrawable::TexturedDrawable() " << texture << texture->id();
        }
    }

    TexturedDrawable::TexturedDrawable(Object3D &object,
                                       GL::Texture2D *texture,
                                       GameShader &shader,
                                       GL::Mesh &mesh,
                                       SceneGraph::DrawableGroup3D &drawables,
                                       UnsignedInt objectId)
        : SceneGraph::Drawable3D{object, &drawables},
            _object(object),
          _color{0xffffffff_rgbaf},
          _ownTexture{Containers::NullOpt},
          _texture(texture),
          _mesh{mesh},
    _phongShader(nullptr),
          _gameShader{&shader},
          _objectId(objectId) {
        if (_texture) {
            Debug{} << "TexturedDrawable::TexturedDrawable() " << texture << texture->id();
        }
    }

    GL::Texture2D TexturedDrawable::makeTexture(const Trade::ImageData2D &image) {
        GL::Texture2D t{};
        t.setWrapping(GL::SamplerWrapping::ClampToEdge)
                .setMagnificationFilter(GL::SamplerFilter::Linear)
                .setMinificationFilter(GL::SamplerFilter::Linear)
                .setStorage(1, GL::textureFormat(image.format()), image.size())
                .setSubImage(0, {}, image);
        return t;
    }

    void TexturedDrawable::draw(const Matrix4 &transformation, SceneGraph::Camera3D &camera) {
        if (_color.a() <= 0.0f) return;
        CHECK_GL_ERROR();
        if (_phongShader) {
            auto& _shader = *_phongShader;
            _shader.setProjectionMatrix(camera.projectionMatrix());
            _shader.setTransformationMatrix(transformation);
            _shader.setDiffuseColor(_color);
            // _shader.setLightRanges({Constants::inf()});
            _shader.setAmbientColor({ambientColor, ambientColor, ambientColor, 1.0f});
            _shader.setLightColors({Color3{lightColor, lightColor, lightColor}});
            _shader.setSpecularColor(_color);
            _shader.setShininess(shininess);
            _shader.setSpecularColor({specular, specular,specular,1.0f});
            _shader.setLightPositions({object().absoluteTransformationMatrix().inverted() * Vector4{lightDirection, 0.0f}.normalized()});
            if (_texture) {
                _shader.bindDiffuseTexture(*_texture);
                CHECK_GL_ERROR();
            }
            if (_shader.flags() & Shaders::PhongGL::Flag::ObjectId) {
                _shader.setObjectId(_objectId);
            }
            if (_shader.flags() & Shaders::PhongGL::Flag::DynamicPerVertexJointCount) {
                if (_skinMeshDrawable.boneMatrices != nullptr) {
                    _shader.setPerVertexJointCount(_skinMeshDrawable.perVertexJointCount, _skinMeshDrawable.secondaryPerVertexJointCount);
                    CHECK_GL_ERROR();
                    _shader.setJointMatrices(*_skinMeshDrawable.boneMatrices);
                    CHECK_GL_ERROR();
                } else {
                    _shader.setPerVertexJointCount(0, 0);
                }
            }
            CHECK_GL_ERROR();

            _shader.draw(_mesh);
        }
        else if (_gameShader) {

            auto& _shader = *_gameShader;
            _shader.setProjectionMatrix(camera.projectionMatrix());
            _shader.setTransformationMatrix(transformation);
            _shader.setNormalMatrix(transformation.rotation());
            _shader.setSpecularColor(_color.rgb());
            _shader.setModelMatrix(object().absoluteTransformationMatrix());
            _shader.setLightVector(lightDirection);
            _shader.setShininess(shininess);
            _shader.setLightColor({lightColor, lightColor, lightColor});
            _shader.setAmbientColor({ambientColor, ambientColor, ambientColor});
            if (_texture) {
                _shader.setDiffuseTexture(*_texture);
                CHECK_GL_ERROR();
            }
            if (_skinMeshDrawable.boneMatrices != nullptr) {
                _shader.setPerVertexJointCount(_skinMeshDrawable.perVertexJointCount);
                CHECK_GL_ERROR();
                _shader.setJointMatrices(*_skinMeshDrawable.boneMatrices);
            } else {
                _shader.setPerVertexJointCount(0);
            }
            CHECK_GL_ERROR();

            _shader.draw(_mesh);
        }
        CHECK_GL_ERROR();
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
        Debug{} << "TexturedDrawable setSkin boneMatrices" << &skin.boneMatrices() << "data @" << skin.boneMatrices().
                data() << "size" << skin.boneMatrices().size();
        _skinMeshDrawable = {
            &skin.boneMatrices(),
            perVertexJointCount,
            secondaryPerVertexJointCount
        };
    }
}
