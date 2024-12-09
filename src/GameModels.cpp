#include "GameModels.h"
#include <memory>
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
#include <Magnum/ImageView.h>
#include <Corrade/Utility/Path.h>

#include "MagnumGameApp.h"

namespace MagnumGame {


    GameModels::GameModels(Trade::AbstractImporter &gltfImporter) {

        if (auto modelsDir = MagnumGameApp::findDirectory("models/characters")) {
            auto filePath = Utility::Path::join(*modelsDir, "character-female-b.glb");
            if (gltfImporter.openFile(filePath)) {
                for (UnsignedInt sc = 0; sc < gltfImporter.sceneCount(); sc++) {
                    Debug{} << "Scene" << sc << ":" << gltfImporter.sceneName(sc) << "(default"
                            << gltfImporter.defaultScene() << ")";
                    auto sceneData = gltfImporter.scene(sc);
//                    loadModel(gltfImporter, *sceneData, "Player", &_playerMesh, nullptr, nullptr, nullptr);
                }
              }
        }
    }

    void GameModels::loadModel(Trade::AbstractImporter& gltfImporter,
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




    std::vector<GL::Texture2D> GameModels::loadTextures(Trade::AbstractImporter &importer) {
        std::vector<GL::Texture2D> textures;
        Debug{} << "Textures:" << importer.textureCount();
        for (auto textureId = 0; textureId < importer.textureCount(); textureId++) {
            auto textureData = importer.texture(textureId);
            auto textureName = importer.textureName(textureId);
            Debug debug{};
            debug << "Texture" << textureId << textureName;
            auto &texture = textures.emplace_back();
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

    std::vector<GL::Texture2D *> GameModels::loadMaterials(Trade::AbstractImporter &importer, std::vector<GL::Texture2D>& textures) {
        std::vector<GL::Texture2D *> materialTextures{};
        Debug{} << "Materials:" << importer.materialCount();
        for (auto materialId = 0; materialId < importer.materialCount(); materialId++) {
            auto material = importer.material(materialId);
            Debug{} << "\tMaterial" << materialId << importer.materialName(materialId) << material->types();
            for (auto attributeId = 0; attributeId < material->attributeCount(); attributeId++) {
                Debug{} << "\t\tattribute" << attributeId << material->attributeName(attributeId) << material->
                        attributeType(attributeId) << material->attributeDataFlags();
            }

            GL::Texture2D *texId = nullptr;
            if (auto textureId = material->findAttribute<UnsignedInt>(Trade::MaterialAttribute::DiffuseTexture)) {
                texId = &textures[*textureId];
            } else if ((textureId = material->findAttribute<UnsignedInt>(Trade::MaterialAttribute::BaseColorTexture))) {
                texId = &textures[*textureId];
            }
            materialTextures.emplace_back(texId);
        }
        return materialTextures;
    }
}