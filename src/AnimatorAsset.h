#pragma once

#include <map>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Trade/AbstractImporter.h>
#include "MagnumGameCommon.h"


namespace MagnumGame {



    struct MaterialAsset {
        GL::Texture2D* texture;
    };

    struct AnimatorAsset {

        struct Bone {
            int boneId;
            Containers::String name;
            Matrix4 transform;
            Containers::Array<Bone> children;

            explicit Bone(const Containers::String &name)
                : name(name) {
            }
            DISALLOW_COPY(Bone)
        };

        struct SkinAsset {
            Containers::Array<UnsignedInt> _jointBoneIds;
            Containers::Array<Matrix4> _inverseBindMatrices{};

            explicit SkinAsset(const Containers::ArrayView<const UnsignedInt> &joints,
                const Containers::ArrayView<const Matrix4>& inverseBindMatrices
                                        );
            DISALLOW_COPY(SkinAsset)
        };

        struct SkinMeshAsset {
            SkinAsset* skin;
            GL::Mesh* mesh;
            MaterialAsset* material;
            UnsignedInt perVertexJointCounts;
            UnsignedInt perVertexJointCountsSecondary;
        };

        struct SkinMeshNode {
            Containers::String name;
            Matrix4 transform{Math::IdentityInit};
            SkinMeshAsset skinMesh{};
            Containers::Array<SkinMeshNode> children{};

            explicit SkinMeshNode(const Containers::String &name): name(name) {}
        };

        //Animation asset data
        Containers::Array<GL::Mesh> _meshes{};
        Containers::Array<GL::Texture2D> _textures{};
        Containers::Array<MaterialAsset> _materials{};
        Containers::Array<SkinAsset> _skins{};
        SkinMeshNode _rootSkinMeshNode{"ROOT"};
        std::map<Containers::String,Trade::AnimationData> _animations{};
        Bone _rootBone{"ROOT"};
        std::map<int,Bone&> _bonesById{};

        explicit AnimatorAsset(Trade::AbstractImporter &importer);
    };

} // MagnumGame

