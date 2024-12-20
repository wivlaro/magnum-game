#include "GameAssets.h"
#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/MaterialData.h>
#include <Magnum/Trade/TextureData.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>
#include <Corrade/Utility/Path.h>

#include "GameShader.h"
#include "MagnumGameApp.h"
#include "ShadowCasterShader.h"

namespace MagnumGame {

    using namespace Magnum::Math::Literals;

    static Containers::Optional<Containers::String> findDirectory(Containers::StringView dirName) {
        using namespace Corrade::Utility;

        if (auto currentDir = Path::currentDirectory()) {
            auto dir = *currentDir;
            while (true) {
                auto candidateDir = Path::join(dir, dirName);
                if (Path::exists(candidateDir) && Path::isDirectory(candidateDir)) {
                    Debug{} << "Found" << candidateDir << "directory";
                    return candidateDir;
                }
                auto parentDir = Path::split(dir).first();
                if (dir == parentDir) {
                    break;
                }
                dir = parentDir;
            }
        }
        Error{} << "Found no" << dirName << "directory, looking in upwards from current" << Path::currentDirectory();
        return {};
    }

    GameAssets::GameAssets(Trade::AbstractImporter& importer){

        _modelsDir = *findDirectory("models");
        _fontsDir = *findDirectory("font");
        _shadersDir = *findDirectory("shaders");

        _shadowCasterShader.emplace(
            Utility::Path::join(_shadersDir, "ShadowCaster.vert"),
            Utility::Path::join(_shadersDir, "ShadowCaster.frag"), 0);

        _animatedShadowCasterShader.emplace(
            Utility::Path::join(_shadersDir, "ShadowCaster.vert"),
            Utility::Path::join(_shadersDir, "ShadowCaster.frag"), MaxAnimationBones);

        _texturedShader.emplace(
            Utility::Path::join(_shadersDir, "GameShader.vert"),
            Utility::Path::join(_shadersDir, "GameShader.frag"), 0, ShadowMapLevels, ShadowPercentageCloserFiltering);

        _animatedTexturedShader.emplace(
            Utility::Path::join(_shadersDir, "GameShader.vert"),
            Utility::Path::join(_shadersDir, "GameShader.frag"), MaxAnimationBones, ShadowMapLevels, ShadowPercentageCloserFiltering);
        _animatedTexturedShader->setAmbientColor(0x111111_rgbf);

        _vertexColorShader.emplace();

        _playerAsset = loadAnimatedModel(importer, "characters/character-female-b.glb");

    }

    GameAssets::~GameAssets() = default;

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
                if (auto matOpt = gltfImporter.material(meshMat.second())) {
                    if (auto textureId = matOpt->findAttribute<UnsignedInt>(Trade::MaterialAttribute::BaseColorTexture)) {
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
#ifndef MAGNUM_TARGET_WEBGL
            texture.setLabel(textureName);
#endif
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
            // for (auto attributeId = 0U; attributeId < material->attributeCount(); attributeId++) {
            //     Debug{} << "\t\tattribute" << attributeId << material->attributeName(attributeId) << material->
            //             attributeType(attributeId) << material->attributeDataFlags();
            // }

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

        auto filePath = Utility::Path::join(_modelsDir, fileName);
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

        return Containers::Pointer<AnimatorAsset>(InPlaceInit, importer);
    }
}