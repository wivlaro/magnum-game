#pragma once
#include <vector>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/Math/Vector3.h>

#include "GameAssets.h"
#include "MagnumGameApp.h"

namespace MagnumGame {
    class Player;

    class GameState {
    public:
        GameState(Timeline& timeline, GameAssets& assets);

        void start();
        void update();

        void setupPlayer();

        Player* getPlayer() { return _player.get(); }

        btCollisionWorld& getWorld() { return _bWorld; }

        void renderDebug(const Matrix4& matrix4);

        void setControl(Vector2 vector2);

        void loadLevel(Trade::AbstractImporter& importer);

        void drawOpaque();

        void drawTransparent();

        CameraController* getCamera() { return _cameraController.get(); }

    private:
        const Timeline& _timeline;
        GameAssets& _assets;

        btDbvtBroadphase _bBroadphase;
        btDefaultCollisionConfiguration _bCollisionConfig;
        btCollisionDispatcher _bDispatcher{&_bCollisionConfig};
        btSequentialImpulseConstraintSolver _bSolver;
        BulletIntegration::DebugDraw _debugDraw{NoCreate};

        /* The world has to live longer than the scene because RigidBody
           instances have to remove themselves from it on destruction */
        btDiscreteDynamicsWorld _bWorld{&_bDispatcher, &_bBroadphase, &_bSolver, &_bCollisionConfig};

        std::vector<Containers::Pointer<GL::Mesh>> _levelMeshes{};
        std::vector<Containers::Pointer<btConvexHullShape>> _levelShapes{};
        Containers::Array<GL::Texture2D> _levelTextures{};
        Containers::Array<MaterialAsset> _levelMaterials{};
        std::map<GL::Mesh*, btConvexHullShape*> _meshToShapeMap{};

        SceneGraph::DrawableGroup3D _animatorDrawables{};
        SceneGraph::DrawableGroup3D _opaqueDrawables{};
        SceneGraph::DrawableGroup3D _transparentDrawables{};

        Scene3D _scene;
        Containers::Pointer<CameraController> _cameraController;

        Containers::Pointer<Player> _player;

        bool _isStarted = false;

        btSphereShape _bSphereQueryShape{0.4f};
        btGhostObject _bSphereQueryObject;
    };
}
