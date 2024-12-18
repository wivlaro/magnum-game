#pragma once

#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>

#include <Magnum/GL/Mesh.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <Magnum/Trade/Trade.h>

#include "Animator.h"

namespace MagnumGame {
    class ShadowCasterShader;

    using namespace Magnum;

    class GameAssets {
public:
        static constexpr int ShadowMapLevels = 2;
        static constexpr bool ShadowPercentageCloserFiltering = true;
        static constexpr int MaxAnimationBones = 16;
        static constexpr Vector2i ShadowMapResolution = {1024, 1024};

        explicit GameAssets(Trade::AbstractImporter& );
        ~GameAssets();

        static Containers::Array<GL::Texture2D> loadTextures(Trade::AbstractImporter &importer);
        static Containers::Array<MaterialAsset> loadMaterials(Trade::AbstractImporter &importer, Containers::Array<GL::Texture2D>& textures);

        Containers::Pointer<AnimatorAsset> loadAnimatedModel(Trade::AbstractImporter &importer,
                                                                    Containers::StringView fileName);

        auto& getPlayerShape() { return _bPlayerShape; }
        auto getPlayerAsset() { return _playerAsset.get(); }

        auto& getShadowCasterShader() { return *_shadowCasterShader; }
        auto& getAnimatedShadowCasterShader() { return *_animatedShadowCasterShader; }
        auto& getAnimatedTexturedShader() { return *_animatedTexturedShader; }
        auto& getTexturedShader() { return *_texturedShader; }
        auto& getVertexColorShader() { return *_vertexColorShader; }

        Containers::StringView getModelsDir() const { return _modelsDir; }

        Containers::StringView getFontsDir() const { return _fontsDir; }

    private:

        Containers::String _modelsDir;
        Containers::String _fontsDir;
        Containers::String _shadersDir;

        Containers::Pointer<ShadowCasterShader> _shadowCasterShader{};
        Containers::Pointer<ShadowCasterShader> _animatedShadowCasterShader{};
        Containers::Pointer<GameShader> _texturedShader{};
        Containers::Pointer<GameShader> _animatedTexturedShader{};
        Containers::Pointer<Shaders::VertexColorGL3D> _vertexColorShader{};

        btStaticPlaneShape _bGroundShape{{0,1,0},0};
        btCapsuleShape _bPlayerShape{0.125, 0.5};
        Containers::Pointer<AnimatorAsset> _playerAsset{};


        void loadModel(Trade::AbstractImporter &gltfImporter, Trade::SceneData &sceneData,
                       Containers::StringView objectName, GL::Mesh *outMesh, Matrix4x4 *outTransform,
                       std::shared_ptr<GL::Texture2D> *outTexture, btConvexHullShape *outConvexHullShape);
    };
}


