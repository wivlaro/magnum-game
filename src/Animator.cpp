//
// Created by Bill Robinson on 07/12/2024.
//

#include "Animator.h"

#include <unordered_set>
#include <set>
#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/AnimationData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/SkinData.h>
#include <Corrade/Containers/StructuredBindings.h>
#include <Magnum/Math/CubicHermite.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Trade/MeshData.h>

#include "GameModels.h"
#include "TexturedDrawable.h"

namespace MagnumGame {
    Animator::Animator(Object3D &rootObject, Trade::AbstractImporter &importer, Shaders::PhongGL& meshShader, SceneGraph::DrawableGroup3D *animDrawables, SceneGraph::DrawableGroup3D *meshDrawables)
        : SceneGraph::Drawable3D(rootObject, animDrawables),
    _fakeBoneCamera{_boneScene}
    {
        auto sceneId = importer.defaultScene();
        Debug{} << "Scene" << sceneId << ":" << importer.sceneName(sceneId);
        auto sceneData = importer.scene(sceneId);

        for (auto skinId = 0; skinId < importer.skin3DCount(); skinId++) {
            Debug{} << "Skin" << skinId << importer.skin3DName(skinId) << "Allocated";

            auto skinData = importer.skin3D(skinId);
            auto inverseBindMatrices = skinData->inverseBindMatrices();

            _skins.emplace_back(inverseBindMatrices);
        }

        _textures = std::move(GameModels::loadTextures(importer));
        auto materialTextures = GameModels::loadMaterials(importer, _textures);

        std::map<int, BoneObject *> boneMap;
        auto &rootBone = getRootBone();

        std::set<int> meshTree{};
        std::function<bool(int, BoneObject &, int)> processBones = [&](int parentId, BoneObject &parentBone, int depth) {
            boneMap[parentId] = &parentBone;
            bool usedByMesh = parentId>= 0 && !sceneData->meshesMaterialsFor(parentId).isEmpty();

            Debug{} << std::string(depth, '\t') << "Bone" << parentId <<  (parentId == -1 ? "ROOT" : importer.objectName(parentId)) << (
                usedByMesh ? "HAS MESH" : "");

            for (auto &childId: sceneData->childrenFor(parentId)) {
                auto &childBone = parentBone.addChild<BoneObject>();

                if (auto transform = sceneData->transformation3DFor(childId)) {
                    childBone.setTransformation(*transform);
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

        processBones(-1, rootBone, 0);

        for (auto skinId = 0; skinId < importer.skin3DCount(); skinId++) {
            Debug{} << "Skin" << skinId << importer.skin3DName(skinId) << "Add joints";

            auto skinData = importer.skin3D(skinId);
            auto joints = skinData->joints();
            auto inverseBindMatrices = skinData->inverseBindMatrices();
            auto &skin = _skins[skinId];

            for (auto jointIndex = 0; jointIndex < joints.size(); jointIndex++) {
                auto jointId = joints[jointIndex];
                Debug{} << "Joint" << jointIndex << jointId << importer.objectName(jointId);

                boneMap[jointId]->addFeature<JointDrawable>(inverseBindMatrices[jointIndex],
                                                            skin.boneMatrices()[jointIndex], _jointDrawables);
            }
        }

        Debug{} << "Meshes:" << importer.meshCount();
        Containers::Array<Containers::Pair<UnsignedInt, UnsignedInt>> perVertexJointCounts;
        for (auto meshId = 0; meshId < importer.meshCount(); meshId++) {
            auto meshName = importer.meshName(meshId);
            Debug{} << "\tMesh" << meshId << ":" << meshName;
            auto meshData = importer.mesh(meshId);

            auto& mesh = _meshes.emplace_back(MeshTools::compile(*meshData));
            mesh.setLabel(meshName);
            arrayAppend(perVertexJointCounts, MeshTools::compiledPerVertexJointCount(*meshData));
        }

        std::map<int, Object3D *> objectMap;
        std::function<void(int, Object3D &, int)> processMeshes = [&](int parentId, Object3D &parent, int depth) {
            if (meshTree.find(parentId) == meshTree.end()) {
                Debug{} << std::string(depth, '\t') << "Skipping" << parentId << (parentId == -1 ? "ROOT" : importer.objectName(parentId))
                << "No Mesh";
                return;
            }

            objectMap[parentId] = &parent;

            for (auto &childId: sceneData->childrenFor(parentId)) {
                auto childName = importer.objectName(childId);
                Debug{} << std::string(depth, '\t') << "Object" << childId << childName;

                auto &child = parent.addChild<Object3D>();

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

                    auto& drawable = child.addFeature<TexturedDrawable>(materialTextures[matId], meshShader, _meshes[meshId], *meshDrawables);
                    drawable.setSkin(_skins[skinId], perVertexJointCounts[meshId].first(), perVertexJointCounts[meshId].second());
                }

                processMeshes(childId, child, depth + 1);
            }
        };

        processMeshes(-1, rootObject, 0);

        if (importer.animationCount() > 0) {
            for (UnsignedInt animationIndex = 0; animationIndex != importer.animationCount(); ++animationIndex) {
                auto animationName = importer.animationName(animationIndex);
                Debug{} << "Animation" << animationIndex << animationName;
                auto animationData = importer.animation(animationIndex);
                auto &animation = *animationData;

                auto &player = _animations[animationName];

                for (UnsignedInt trackIndex = 0; trackIndex != animation.trackCount(); ++trackIndex) {
                    auto targetObjectId = animation.trackTarget(trackIndex);
                    Debug{} << "\tTrack" << trackIndex << importer.objectName(targetObjectId) << animation.
                            trackTargetName(trackIndex) <<
                            animation.trackType(trackIndex);

                    auto targetObject = boneMap.find(targetObjectId);

                    if (targetObject == boneMap.end()) {
                        Warning{} << "Can't find bone for animation track" << trackIndex << targetObjectId;
                        continue;
                    }

                    auto &animatedObject = *targetObject->second;

                    switch (animation.trackTargetName(trackIndex)) {
                        case Trade::AnimationTrackTarget::Translation3D: {
                            const auto callback = [](Float, const Vector3 &translation, auto &object) {
                                object.setTranslation(translation);
                            };
                            if (animation.trackType(trackIndex) == Trade::AnimationTrackType::CubicHermite3D) {
                                player.addWithCallback(animation.track<CubicHermite3D>(trackIndex),
                                                       callback, animatedObject);
                            } else {
                                CORRADE_INTERNAL_ASSERT(
                                    animation.trackType(trackIndex) == Trade::AnimationTrackType::Vector3);
                                player.addWithCallback(animation.track<Vector3>(trackIndex),
                                                       callback, animatedObject);
                            }
                        }
                        break;
                        case Trade::AnimationTrackTarget::Rotation3D: {
                            const auto callback = [](Float, const Quaternion &rotation, auto &object) {
                                object.setRotation(rotation);
                            };
                            if (animation.trackType(trackIndex) == Trade::AnimationTrackType::CubicHermiteQuaternion) {
                                player.addWithCallback(animation.track<CubicHermiteQuaternion>(trackIndex),
                                                       callback, animatedObject);
                            } else {
                                CORRADE_INTERNAL_ASSERT(
                                    animation.trackType(trackIndex) == Trade::AnimationTrackType::Quaternion);
                                player.addWithCallback(animation.track<Quaternion>(trackIndex),
                                                       callback, animatedObject);
                            }
                        }
                        break;
                        case Trade::AnimationTrackTarget::Scaling3D: {
                            const auto callback = [](Float, const Vector3 &scaling, auto &object) {
                                object.setScaling(scaling);
                            };
                            if (animation.trackType(trackIndex) == Trade::AnimationTrackType::CubicHermite3D) {
                                player.addWithCallback(animation.track<CubicHermite3D>(trackIndex),
                                                       callback, animatedObject);
                            } else {
                                CORRADE_INTERNAL_ASSERT(
                                    animation.trackType(trackIndex) == Trade::AnimationTrackType::Vector3);
                                player.addWithCallback(animation.track<Vector3>(trackIndex),
                                                       callback, animatedObject);
                            }
                        }
                        break;
                        default: CORRADE_INTERNAL_ASSERT_UNREACHABLE();
                            break;
                    }
                }

                player.setDuration(animationData->duration());
                player.setPlayCount(0);
                arrayAppend(_animationData, animation.release());
            }
        }
    }

    Skin &Animator::getSkin(size_t skinIndex) {
        return _skins[skinIndex];
    }

    Skin::Skin(const Containers::ArrayView<const Matrix4> &inverseBindMatrices)
        : _boneMatrices{DirectInit, inverseBindMatrices.size(), Math::IdentityInit},
          _inverseBindMatrices{InPlaceInit, inverseBindMatrices} {
    }

    void Animator::draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) {
        if (_currentAnimation) {
            _currentAnimation->advance(std::chrono::system_clock::now().time_since_epoch());
        }
        _fakeBoneCamera.draw(_jointDrawables);
    }

    void Animator::play(const Containers::StringView &animationName, bool restart) {
        auto animIt = _animations.find(animationName);
        if (animIt == _animations.end()) {
            Warning{} << "Can't find animation" << animationName;
            return;
        }

        if (!restart && &animIt->second == _currentAnimation) {
            return;
        }

        if (_currentAnimation) {
            _currentAnimation->stop();
        }
        _currentAnimation = &_animations[animationName];
        _currentAnimation = &animIt->second;
        assert(_currentAnimation == &animIt->second);
        _currentAnimation->play(std::chrono::system_clock::now().time_since_epoch());
    }

    void Animator::addAnimation(const Containers::String &name, AnimationPlayer &&player) {
        _animations[name] = std::move(player);
    }
} // MagnumGame
