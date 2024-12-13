#include "GameAssets.h"
#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/MaterialData.h>
#include <Magnum/Trade/TextureData.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>
#include <Magnum/ImageView.h>
#include <Corrade/Utility/Path.h>

#include "MagnumGameApp.h"

namespace MagnumGame {

    using namespace Magnum::Math::Literals;

    GameAssets::GameAssets(Trade::AbstractImporter& importer) {

        _unlitAlphaShader = Shaders::FlatGL3D{Shaders::FlatGL3D::Configuration{}.setFlags(Shaders::FlatGL3D::Flag::Textured)};

        _texturedShader = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
            .setFlags(Shaders::PhongGL::Flag::DiffuseTexture | Shaders::PhongGL::Flag::ObjectId )};
        _texturedShader.setAmbientColor(0x111111_rgbf)
                .setSpecularColor(0x33000000_rgbaf)
                .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});


        _animatedTexturedShader = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
            .setJointCount(16, 4)
            .setFlags(Shaders::PhongGL::Flag::DiffuseTexture | Shaders::PhongGL::Flag::ObjectId | Shaders::PhongGL::Flag::DynamicPerVertexJointCount)};
        _animatedTexturedShader.setAmbientColor(0x111111_rgbf)
                .setSpecularColor(0x33000000_rgbaf)
                .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});

        _vertexColorShader = Shaders::VertexColorGL3D{};
        _flatShader = Shaders::FlatGL3D{};

        _playerAsset = loadAnimatedModel(importer, "characters/character-female-b.glb");
    }

    void GameAssets::loadModel(Trade::AbstractImporter& gltfImporter,
                                  Trade::SceneData &sceneData,
                                  Containers::StringView objectName, GL::Mesh *outMesh, Matrix4x4 *outTransform,
                                  std::shared_ptr<GL::Texture2D> *outTexture, btConvexHullShape *outConvexHullShape) {
        auto objectId = gltfImporter.objectForName(objectName);
        for (auto &meshMat: sceneData.meshesMaterialsFor(objectId)) {
            auto meshId = meshMat.first();
            auto meshName = gltfImporter.meshName(meshId);
            auto materialId = meshMat.second();
            auto materialName = gltfImporter.materialName(materialId);
            Debug{} << "loadModel Object" << objectName << "has pOutMesh" << meshId << meshName << "and material"
                    << materialId << materialName;
            if (outMesh) {
                *outMesh = MeshTools::compile(*gltfImporter.mesh(meshId));
                if (outConvexHullShape) {
                    auto meshPositions = gltfImporter.mesh(meshId)->positions3DAsArray();
                    *outConvexHullShape = btConvexHullShape{
                        meshPositions.data()->data(), static_cast<int>(meshPositions.size()),
                        sizeof(Vector3)
                    };
                }
            }
            if (outTransform) {
                if (auto transformOpt = sceneData.transformation3DFor(objectId)) {
                    *outTransform = *transformOpt;
                }
            }
            if (outTexture) {
                auto matOpt = gltfImporter.material(meshMat.second());
                if (matOpt) {
                    auto textureId = matOpt->findAttribute<UnsignedInt>(Trade::MaterialAttribute::BaseColorTexture);
                    if (textureId) {
                        Debug{} << "loadModel Material" << materialName << "has base color texture" << *textureId
                                << "with name" << gltfImporter.textureName(*textureId);
                        if (auto textureOpt = gltfImporter.texture(*textureId)) {
                            if (auto imageOpt = gltfImporter.image2D(textureOpt->image())) {
                                (*outTexture = std::make_shared<GL::Texture2D>())
                                        ->setWrapping(GL::SamplerWrapping::ClampToEdge)
                                        .setMagnificationFilter(GL::SamplerFilter::Linear)
                                        .setMinificationFilter(GL::SamplerFilter::Linear)
                                        .setStorage(1, GL::textureFormat(imageOpt->format()), imageOpt->size())
                                        .setSubImage(0, {}, *imageOpt);
                            }
                        }
                    }
                }
            }
            break;
        }
    }


    Containers::Array<GL::Texture2D> GameAssets::loadTextures(Trade::AbstractImporter &importer) {
        Containers::Array<GL::Texture2D> textures;
        arrayReserve(textures, importer.textureCount());
        Debug{} << "Textures:" << importer.textureCount();
        for (auto textureId = 0U; textureId < importer.textureCount(); textureId++) {
            auto textureData = importer.texture(textureId);
            auto textureName = importer.textureName(textureId);
            Debug debug{};
            debug << "Texture" << textureId << textureName;
            auto &texture = arrayAppend(textures, InPlaceInit);
            debug << "id=" << texture.id();
            texture.setLabel(textureName);
            auto image = importer.image2D(textureData->image());
            if (image) {
                debug << image->size();
                texture.setMagnificationFilter(textureData->magnificationFilter())
                        .setMinificationFilter(textureData->minificationFilter(), textureData->mipmapFilter())
                        .setWrapping(textureData->wrapping().xy())
                        .setStorage(1, GL::textureFormat(image->format()), image->size())
                        .setSubImage(0, {}, *image);
            }
        }
        return textures;
    }

    Containers::Array<MaterialAsset> GameAssets::loadMaterials(Trade::AbstractImporter &importer, Containers::Array<GL::Texture2D>& textures) {
        Containers::Array<MaterialAsset> materials{};
        arrayReserve(materials, importer.materialCount());
        Debug{} << "Materials:" << importer.materialCount();
        for (auto materialId = 0U; materialId < importer.materialCount(); materialId++) {
            auto material = importer.material(materialId);
            Debug{} << "\tMaterial" << materialId << importer.materialName(materialId) << material->types();
            for (auto attributeId = 0U; attributeId < material->attributeCount(); attributeId++) {
                Debug{} << "\t\tattribute" << attributeId << material->attributeName(attributeId) << material->
                        attributeType(attributeId) << material->attributeDataFlags();
            }

            GL::Texture2D *texture = nullptr;
            if (auto textureId = material->findAttribute<UnsignedInt>(Trade::MaterialAttribute::DiffuseTexture)) {
                texture = &textures[*textureId];
            } else if ((textureId = material->findAttribute<UnsignedInt>(Trade::MaterialAttribute::BaseColorTexture))) {
                texture = &textures[*textureId];
            }
            arrayAppend(materials, InPlaceInit, texture);
            Debug{} << "\tMaterial" << materialId << " texture ID" << texture->id() << " address " << materials[materialId].texture;
        }
        return materials;
    }




    Containers::Pointer<AnimatorAsset> GameAssets::loadAnimatedModel(Trade::AbstractImporter &importer, Containers::StringView fileName) {

        if (auto modelsDir = MagnumGameApp::findDirectory("models")) {
            auto filePath = Utility::Path::join(*modelsDir, fileName);
            if (!importer.openFile(filePath)) {
                Error{} << "Can't open" << filePath << "with" << importer.plugin();
                return {};
            }
            if (importer.sceneCount() == 0) {
                Error{} << "No scene found in" << filePath;
                importer.close();
                return {};
            }
            if (importer.sceneCount() > 1) {
                Warning{} << "Multiple scenes found in" << filePath << ", using the default one" << importer.defaultScene() << importer.sceneName(importer.defaultScene());
            }
        }

        return Containers::Pointer<AnimatorAsset>(InPlaceInit, importer);
    }
}