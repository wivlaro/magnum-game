//
// Created by Bill Robinson on 12/12/2024.
//

#include "AnimatorAsset.h"
#include <set>
#include <ranges>
#include <assert.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/StructuredBindings.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/SkinData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/MeshTools/Compile.h>

#include "GameAssets.h"

namespace MagnumGame {
    AnimatorAsset::SkinAsset::SkinAsset(const Containers::ArrayView<const UnsignedInt> &joints,
                                        const Containers::ArrayView<const Matrix4> &inverseBindMatrices)
        : _jointBoneIds(InPlaceInit, joints),
          _inverseBindMatrices(InPlaceInit, inverseBindMatrices) {
    }

    AnimatorAsset::AnimatorAsset(Trade::AbstractImporter &importer)
        : _meshes(DefaultInit, importer.meshCount())
          , _textures(GameAssets::loadTextures(importer))
          , _materials(GameAssets::loadMaterials(importer, _textures))
          , _skins{NoInit, importer.skin3DCount()} {
        auto sceneId = importer.defaultScene();
        Debug{} << "Scene" << sceneId << ":" << importer.sceneName(sceneId);
        auto sceneData = importer.scene(sceneId);

        std::set<int> meshTree{};
        std::function<bool(int, Bone &, int)> processBones = [&](int parentId, Bone &parentBone, int depth) {
            assert(_bonesById.find(parentId) == _bonesById.end());
            _bonesById.insert({parentId, parentBone});
            assert(_bonesById.find(parentId) != _bonesById.end());
            assert(&_bonesById.find(parentId)->second == &parentBone);

            parentBone.boneId = parentId;

            bool usedByMesh = parentId >= 0 && !sceneData->meshesMaterialsFor(parentId).isEmpty();

            Debug{} << std::string(depth, '\t') << "Bone" << parentId
                    << (parentId == -1 ? "ROOT" : importer.objectName(parentId))
                    << (usedByMesh ? "HAS MESH" : "");

            std::vector<Bone> newBones;

            auto childIds = sceneData->childrenFor(parentId);
            parentBone.children = Containers::Array<Bone>{NoInit, childIds.size()};
            for (size_t childIndex = 0; childIndex < childIds.size(); ++childIndex) {
                auto childId = childIds[childIndex];
                auto &childBone = *new(&parentBone.children[childIndex]) Bone(importer.objectName(childId));


                if (auto transform = sceneData->transformation3DFor(childId)) {
                    childBone.defaultTranslation = transform->translation();
                    childBone.defaultRotation = Quaternion::fromMatrix(transform->rotation());
                    childBone.defaultScale = transform->scaling();
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
            new(&_skins[skinId]) SkinAsset{jointIds, inverseBindMatrices};
        }

        Debug{} << "Meshes:" << importer.meshCount();
        Containers::Array<Containers::Pair<UnsignedInt, UnsignedInt> > perVertexJointCounts{importer.meshCount()};
        for (auto meshId = 0U; meshId < importer.meshCount(); meshId++) {
            auto meshName = importer.meshName(meshId);
            Debug{} << "\tMesh" << meshId << ":" << meshName;
            auto meshData = importer.mesh(meshId);

            for (auto attrId = 0U; attrId < meshData->attributeCount(); attrId++) {
                auto attrName = meshData->attributeName(attrId);

                Debug{} << "\tAttribute" << attrId << ":" << attrName
                << "format=" << meshData->attributeFormat(attrId)
                << " offset=" << meshData->attributeOffset(attrId)
                << " stride=" << meshData->attributeStride(attrId)
                << " arraySize=" << meshData->attributeArraySize(attrId)
                << " morphTargetId=" << meshData->attributeMorphTargetId(attrId);
            }

            [[maybe_unused]]
            auto &mesh = _meshes[meshId] = MeshTools::compile(*meshData);
#ifndef MAGNUM_TARGET_WEBGL
            mesh.setLabel(meshName);
#endif
            perVertexJointCounts[meshId] = MeshTools::compiledPerVertexJointCount(*meshData);
        }

        std::function<void(int, SkinMeshNode &, int)> processMeshes = [&](int parentId, SkinMeshNode &parentAsset, int depth) {
            if (meshTree.find(parentId) == meshTree.end()) {
                Debug{} << std::string(depth, '\t') << "Skipping"
                        << parentId << (parentId == -1 ? "ROOT" : importer.objectName(parentId))
                        << "No Mesh";
                return;
            }

            auto childIds = sceneData->childrenFor(parentId);
            parentAsset.children = Containers::Array<SkinMeshNode>{NoInit, childIds.size()};
            for (size_t childIndex = 0; childIndex < childIds.size(); ++childIndex) {
                auto childId = childIds[childIndex];
                auto childName = importer.objectName(childId);
                Debug{} << std::string(depth, '\t') << "Object" << childId << childName;

                auto &child = *new(&parentAsset.children[childIndex]) SkinMeshNode(childName);;
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
                            "Material" << matId << (matId != -1 ? importer.materialName(matId) : "NO MATERIAL");

                    auto meshPerVertexJointCounts = perVertexJointCounts[meshId];
                    child.skinMesh = {
                        &_skins[skinId],
                        &_meshes[meshId],
                        &_materials[matId],
                        meshPerVertexJointCounts.first(),
                        meshPerVertexJointCounts.second()
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
