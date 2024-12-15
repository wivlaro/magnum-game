#pragma once

#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>

#include <Magnum/GL/Mesh.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Trade/Trade.h>

#include "Animator.h"

namespace MagnumGame {

	using namespace Magnum;

    class GameAssets {
public:
        explicit GameAssets(Trade::AbstractImporter& );

        static Containers::Array<GL::Texture2D> loadTextures(Trade::AbstractImporter &importer);
        static Containers::Array<MaterialAsset> loadMaterials(Trade::AbstractImporter &importer, Containers::Array<GL::Texture2D>& textures);

        Containers::Pointer<AnimatorAsset> loadAnimatedModel(Trade::AbstractImporter &importer,
                                                                    Containers::StringView fileName);

        btCapsuleShape& getPlayerShape() { return _bPlayerShape; }

        AnimatorAsset* getPlayerAsset() { return _playerAsset.get(); }

        Shaders::PhongGL& getAnimatedTexturedShader() { return _animatedTexturedShader; }
        Shaders::PhongGL& getTexturedShader() { return _texturedShader; }

        Shaders::VertexColorGL3D& getVertexColorShader() { return _vertexColorShader; }

        Containers::StringView getModelsDir() const { return _modelsDir; }

        Containers::StringView getFontsDir() const { return _fontsDir; }

    private:

        Shaders::PhongGL _texturedShader{NoCreate};
        Shaders::PhongGL _animatedTexturedShader{NoCreate};
        Shaders::VertexColorGL3D _vertexColorShader{NoCreate};

        btStaticPlaneShape _bGroundShape{{0,1,0},0};
        btCapsuleShape _bPlayerShape{0.125, 0.5};
        Containers::Pointer<AnimatorAsset> _playerAsset{};

        Containers::String _modelsDir;
        Containers::String _fontsDir;

        void loadModel(Trade::AbstractImporter &gltfImporter, Trade::SceneData &sceneData,
                              Containers::StringView objectName, GL::Mesh *outMesh, Matrix4x4 *outTransform,
                              std::shared_ptr<GL::Texture2D> *outTexture, btConvexHullShape *outConvexHullShape);
    };
}


