//
// Created by Bill Robinson on 12/12/2024.
//

#include "AnimatorAsset.h"
#include <set>
#include <ranges>
#include <assert.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/SkinData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/MeshTools/Compile.h>
#include <Corrade/Containers/StructuredBindings.h>

#include "GameModels.h"

namespace MagnumGame {
 AnimatorAsset::SkinAsset::SkinAsset(const Containers::ArrayView<const UnsignedInt> &joints,
                                    const Containers::ArrayView<const Matrix4>& inverseBindMatrices)
        : _jointBoneIds(InPlaceInit, joints),
          _inverseBindMatrices(InPlaceInit,inverseBindMatrices)
    {
    }

    AnimatorAsset::AnimatorAsset(Trade::AbstractImporter &importer)
        : _meshes(DefaultInit, importer.meshCount())
    , _textures(GameModels::loadTextures(importer))
    , _materials(GameModels::loadMaterials(importer, _textures))
    , _skins{NoInit, importer.skin3DCount()}
    {
        auto sceneId = importer.defaultScene();
        Debug{} << "Scene" << sceneId << ":" << importer.sceneName(sceneId);
        auto sceneData = importer.scene(sceneId);

        std::set<int> meshTree{};
        std::map<int, Bone &> boneMap;
        std::function<bool(int, Bone &, int)> processBones = [&](int parentId, Bone &parentBone, int depth) {
            assert(boneMap.find(parentId) == boneMap.end());
            boneMap.insert({parentId, parentBone});
            assert(boneMap.find(parentId) != boneMap.end());
            assert(&boneMap.find(parentId)->second == &parentBone);

            parentBone.boneId = parentId;

            bool usedByMesh = parentId >= 0 && !sceneData->meshesMaterialsFor(parentId).isEmpty();

            Debug{} << std::string(depth, '\t') << "Bone" << parentId
                    << (parentId == -1 ? "ROOT" : importer.objectName(parentId))
                    << (usedByMesh ? "HAS MESH" : "");

            auto childIds = sceneData->childrenFor(parentId);
            parentBone.children = Containers::Array<Bone>{NoInit, childIds.size()};
            for (size_t childIndex = 0; childIndex < childIds.size(); ++childIndex) {
                auto childId = childIds[childIndex];
                auto& childBone = *new (&parentBone.children[childIndex]) Bone(importer.objectName(childId));

                if (auto transform = sceneData->transformation3DFor(childId)) {
                    childBone.transform = *transform;
                } else {
                    childBone.transform = Matrix4{Math::IdentityInit};
                }

                if (processBones(childId, childBone, depth + 1)) {
                    usedByMesh = true;
                }
            }

            if (usedByMesh) {
                meshTree.insert(parentId);
            }

            return usedByMesh;
        };

        processBones(-1, _rootBone, 0);

        for (auto skinId = 0U; skinId < importer.skin3DCount(); skinId++) {
            Debug{} << "Skin" << skinId << importer.skin3DName(skinId) << "Allocated";

            auto skinData = importer.skin3D(skinId);
            auto inverseBindMatrices = skinData->inverseBindMatrices();

            auto jointIds = skinData->joints();
            assert(jointIds.size() == inverseBindMatrices.size());
            _skins[skinId] = SkinAsset{jointIds, inverseBindMatrices};
        }

        Debug{} << "Meshes:" << importer.meshCount();
        Containers::Array<Containers::Pair<UnsignedInt, UnsignedInt> > perVertexJointCounts{importer.meshCount()};
        for (auto meshId = 0U; meshId < importer.meshCount(); meshId++) {
            auto meshName = importer.meshName(meshId);
            Debug{} << "\tMesh" << meshId << ":" << meshName;
            auto meshData = importer.mesh(meshId);

            auto &mesh = _meshes[meshId] = MeshTools::compile(*meshData);
            mesh.setLabel(meshName);
            perVertexJointCounts[meshId] = MeshTools::compiledPerVertexJointCount(*meshData);
        }

        std::function<void(int, SkinMeshNode&, int)> processMeshes = [&](int parentId, SkinMeshNode& parentAsset, int depth) {

            if (meshTree.find(parentId) == meshTree.end()) {
                Debug{} << std::string(depth, '\t') << "Skipping" << parentId << (parentId == -1
                            ? "ROOT"
                            : importer.objectName(parentId))
                        << "No Mesh";
                return;
            }


            auto childIds = sceneData->childrenFor(parentId);
            parentAsset.children = Containers::Array<SkinMeshNode>{NoInit, childIds.size()};
            for (size_t childIndex = 0; childIndex < childIds.size(); ++childIndex) {
                auto childId = childIds[childIndex];
                auto childName = importer.objectName(childId);
                Debug{} << std::string(depth, '\t') << "Object" << childId << childName;

                auto &child = *new (&parentAsset.children[childIndex]) SkinMeshNode(childName);;
                if (auto transform = sceneData->transformation3DFor(childId)) {
                    child.transform = *transform;
                }

                int skinId = -1;
                for (auto sId: sceneData->skinsFor(childId)) {
                    Debug{} << std::string(depth, '\t') << "  " << "Skin" << sId << importer.skin3DName(sId);
                    skinId = sId;
                    break;
                }
                for (auto [meshId,matId]: sceneData->meshesMaterialsFor(childId)) {

                    Debug{} << std::string(depth, '\t') << "  " << "Mesh" << meshId << importer.meshName(meshId) <<
                            "Material" << matId <<
                            (matId != -1 ? importer.materialName(matId) : "NO MATERIAL");

                    auto meshPerVertexJointCounts = perVertexJointCounts[meshId];
                    child.skinMesh = {
                        .skin = &_skins[skinId],
                        .mesh = &_meshes[meshId],
                        .material = &_materials[matId],
                        .perVertexJointCounts = meshPerVertexJointCounts.first(),
                        .perVertexJointCountsSecondary = meshPerVertexJointCounts.second()
                    };
                }

                processMeshes(childId, child, depth + 1);
            }
        };

        processMeshes(-1, _rootSkinMeshNode, 0);

        if (importer.animationCount() > 0) {
            for (UnsignedInt animationIndex = 0; animationIndex != importer.animationCount(); ++animationIndex) {
                auto animationName = importer.animationName(animationIndex);
                Debug{} << "Animation" << animationIndex << animationName;
                auto animationData = importer.animation(animationIndex);
                _animations.insert(std::pair{animationName, std::move(*animationData)});
            }
        }
    }

} // MagnumGame