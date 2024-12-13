#pragma once

#include <memory>

#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>

#include <Magnum/GL/Mesh.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <Magnum/Trade/Trade.h>

namespace MagnumGame {

	using namespace Magnum;

    class GameModels {
public:
        explicit GameModels(Trade::AbstractImporter& gltfImporter);

        static std::vector<GL::Texture2D> loadTextures(Trade::AbstractImporter &importer);
        static std::vector<GL::Texture2D *> loadMaterials(Trade::AbstractImporter &importer, std::vector<GL::Texture2D>& textures);

        btCapsuleShape& getPlayerShape() { return _bPlayerShape; }

  private:
        Magnum::GL::Mesh _playerMesh{NoCreate};

        btStaticPlaneShape _bGroundShape{{0,1,0},0};
        btCapsuleShape _bPlayerShape{0.125, 0.5};

        static void loadModel(Trade::AbstractImporter &gltfImporter, Trade::SceneData &sceneData,
                              Containers::StringView objectName, GL::Mesh *outMesh, Matrix4x4 *outTransform,
                              std::shared_ptr<GL::Texture2D> *outTexture, btConvexHullShape *outConvexHullShape);
    };
}


