//
// Created by Bill Robinson on 07/12/2024.
//

#include "Animator.h"

#include <ostream>
#include <unordered_set>
#include <set>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/AnimationData.h>
#include <Corrade/Containers/StructuredBindings.h>
#include <Magnum/Math/CubicHermite.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Trade/MeshData.h>

#include "GameModels.h"
#include "TexturedDrawable.h"

namespace MagnumGame {


    Animator::Animator(Object3D &rootObject, const AnimatorAsset &asset, Shaders::PhongGL &meshShader,
                       SceneGraph::DrawableGroup3D *animDrawables, SceneGraph::DrawableGroup3D *meshDrawables)
        : SceneGraph::Drawable3D(rootObject, animDrawables)
    , _fakeBoneCamera{_boneScene}
    , _skins{NoInit, asset._skins.size()}
    {


        std::map<int, BoneObject *> boneMap;

        std::function<void(BoneObject &, const AnimatorAsset::Bone &, int)> instantiateBone =
            [&](BoneObject &parentBone, const AnimatorAsset::Bone &boneAsset, int depth) {

            boneMap[boneAsset.boneId] = &parentBone;
            parentBone.setTransformation(boneAsset.transform);
            Debug{} << std::string(depth, '\t') << "Bone" << boneAsset.name;

            for (auto &childBoneAsset: boneAsset.children) {
                instantiateBone(parentBone.addChild<BoneObject>(), childBoneAsset, depth + 1);
            }
        };

        instantiateBone(getRootBone(), asset._rootBone, 0);

        std::map<const AnimatorAsset::SkinAsset*, Skin*> skinMap;
        for (auto skinIndex = 0; skinIndex < asset._skins.size(); ++skinIndex) {
            auto& skinAsset = asset._skins[skinIndex];
            auto& skin = *new (&_skins[skinIndex]) Skin{skinAsset};
            Debug{} << "Skin setup asset=" << &skinAsset << "skin" << &skin << "matrices" << &skin.boneMatrices() << "data @" << skin.boneMatrices().data() << "size" << skin.boneMatrices().size();
            skinMap[&skinAsset] = &skin;

            for (auto jointIndex = 0U; jointIndex < skinAsset._jointBoneIds.size(); jointIndex++) {
                auto joint = skinAsset._jointBoneIds[jointIndex];
                boneMap[joint]->addFeature<JointDrawable>(skinAsset._inverseBindMatrices[jointIndex], skin.boneMatrices()[jointIndex], _jointDrawables);
            }
        }

        std::map<int, Object3D *> objectMap;
        std::function<void(Object3D &, const AnimatorAsset::SkinMeshNode&)> processMeshes = [&](Object3D &parent, const AnimatorAsset::SkinMeshNode& parentAsset) {

            parent.setTransformation(parentAsset.transform);

            if (parentAsset.skinMesh.mesh != nullptr && parentAsset.skinMesh.material != nullptr) {
                auto& drawable =
                parent.addFeature<TexturedDrawable>(parentAsset.skinMesh.material->texture, meshShader, *parentAsset.skinMesh.mesh, *meshDrawables);

                if (parentAsset.skinMesh.skin != nullptr) {
                    auto skinBone = skinMap.find(parentAsset.skinMesh.skin);
                    if (skinBone != skinMap.end()) {
                        Debug{} << "Skin insta asset=" << parentAsset.skinMesh.skin << "skin" << skinBone->second << "matrices" << &skinBone->second->boneMatrices() << "data @"<< skinBone->second->boneMatrices().data() << "size" << skinBone->second->boneMatrices().size();
                        drawable.setSkin(*skinBone->second,
                            parentAsset.skinMesh.perVertexJointCounts,
                            parentAsset.skinMesh.perVertexJointCountsSecondary);
                    }
                    else {
                        Error{} << "No skin bone found for" << parentAsset.skinMesh.skin;
                    }
                }
            }

            for (auto& child : parentAsset.children) {
                processMeshes(parent.addChild<Object3D>(), child);
            }
        };

        processMeshes(rootObject, asset._rootSkinMeshNode);

        for (auto& [animationName, animationData] : asset._animations) {

            auto &player = _animationPlayers[animationName];

            Debug{} << "Instancing anim" << animationName;
            for (UnsignedInt trackIndex = 0; trackIndex != animationData.trackCount(); ++trackIndex) {

                auto animatedObjectId = animationData.trackTarget(trackIndex);
                auto targetBone = asset._bonesById.find(animatedObjectId);

                if (targetBone != asset._bonesById.end()) {
                    Debug{} << "Track" << animationName << "track" << trackIndex
                    << "target" << animationData.trackTarget(trackIndex)
                    << targetBone->second.name
                    << "type" << animationData.trackType(trackIndex)
                    << "(name" << animationData.trackTargetName(trackIndex) << ")"
                    << "result" << animationData.trackResultType(trackIndex);
                }
                addAnimationTrack(player, animationData, trackIndex, boneMap);
            }

            player.setDuration(animationData.duration());
            player.setPlayCount(0);
        }
    }


    void Animator::addAnimationTrack(
        AnimationPlayer &player, const Trade::AnimationData &animation, UnsignedInt trackIndex,
        std::map<int, BoneObject *> boneMap) {
        auto animatedObjectId = animation.trackTarget(trackIndex);
        auto targetObject = boneMap.find(animatedObjectId);

        if (targetObject == boneMap.end()) {
            Warning{} << "Can't find bone for animation track" << trackIndex << animatedObjectId;
            return;
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

    Skin &Animator::getSkin(size_t skinIndex) {
        return _skins[skinIndex];
    }

    Skin::Skin(const AnimatorAsset::SkinAsset &skinAsset)
        : _boneMatrices{DirectInit, skinAsset._inverseBindMatrices.size(), Math::IdentityInit} {
    }

    void Animator::draw(const Matrix4 &, SceneGraph::Camera3D &) {
        if (_currentAnimation) {
            _currentAnimation->advance(std::chrono::system_clock::now().time_since_epoch());
        }
        _fakeBoneCamera.draw(_jointDrawables);
    }

    void Animator::play(const Containers::StringView &animationName, bool restart) {
        auto animIt = _animationPlayers.find(animationName);
        if (animIt == _animationPlayers.end()) {
            Warning{} << "Can't find animation" << animationName;
            return;
        }

        if (!restart && &animIt->second == _currentAnimation) {
            return;
        }

        if (_currentAnimation) {
            _currentAnimation->stop();
        }
        _currentAnimation = &_animationPlayers[animationName];
        _currentAnimation = &animIt->second;
        assert(_currentAnimation == &animIt->second);
        _currentAnimation->play(std::chrono::system_clock::now().time_since_epoch());
    }

    void Animator::addAnimation(const Containers::String &name, AnimationPlayer &&player) {
        _animationPlayers[name] = std::move(player);
    }
} // MagnumGame