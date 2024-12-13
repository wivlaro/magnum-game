#pragma once

#include <Magnum/Animation/Animation.h>
#include <Magnum/Animation/Player.h>
#include <Magnum/SceneGraph/AbstractFeature.h>
#include <Magnum/SceneGraph/AbstractObject.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Magnum/Math/Matrix4.h>
#include <Corrade/Tags.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Corrade/Containers/String.h>
#include <Corrade/Containers/StringStlHash.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Trade/AnimationData.h>
#include <Magnum/Trade/MaterialData.h>

#include "MagnumGameApp.h"
#include "MagnumGameCommon.h"
#include "AnimatorAsset.h"

namespace MagnumGame {
    using namespace Magnum;

    class Skin;
    struct AnimatorAsset;


    class Animator : public SceneGraph::Drawable3D {
    public:
        typedef Animation::Player<std::chrono::nanoseconds, Float> AnimationPlayer;

        typedef SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation3D> BoneObject;
        typedef SceneGraph::Scene<SceneGraph::TranslationRotationScalingTransformation3D> BoneScene;

        explicit Animator(Object3D &rootObject, const AnimatorAsset &asset, Shaders::PhongGL &meshShader,
                       SceneGraph::DrawableGroup3D *animDrawables, SceneGraph::DrawableGroup3D *meshDrawables);

        Skin& getSkin(size_t skinIndex);

        BoneScene &getRootBone() { return _boneScene; }

        void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) override;

        void play(const Containers::StringView& animationName, bool restart);

        void setDefaultAnimation(const Containers::StringView& animationName);

        void addAnimation(const Containers::String &string, AnimationPlayer &&player);

    private:

        //Animation state data
        BoneScene _boneScene{};
        SceneGraph::Camera3D _fakeBoneCamera;
        SceneGraph::DrawableGroup3D _jointDrawables{};
        Containers::Array<Skin> _skins;

        AnimationPlayer* _currentAnimation{};
        Containers::String _defaultAnimationName;

        std::unordered_map<Containers::String, AnimationPlayer> _animationPlayers{};

        static void addAnimationTrack(AnimationPlayer &player, const Trade::AnimationData &animation, UnsignedInt trackIndex, std::map<int, BoneObject *> boneMap);
    };

    class Skin {
    public:

        explicit Skin(const AnimatorAsset::SkinAsset& skinAsset);

        Containers::Array<Matrix4>& boneMatrices() { return _boneMatrices; }

    private:
        Containers::Array<Matrix4> _boneMatrices{};

    };


    class JointDrawable : public SceneGraph::Drawable3D {
    public:
        explicit JointDrawable(Animator::BoneObject &object, const Matrix4 &inverseBindMatrix, Matrix4 &jointMatrix,
                               SceneGraph::DrawableGroup3D &group)
            : SceneGraph::Drawable3D{object, &group},
              _inverseBindMatrix{inverseBindMatrix},
              /* GCC 4.8 can't handle {} here */
              _jointMatrix(jointMatrix) {
        }

    private:
        void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &) override {
            _jointMatrix = transformationMatrix * _inverseBindMatrix;
        }

        Matrix4 _inverseBindMatrix;
        Matrix4 &_jointMatrix;
    };
} // MagnumGame
