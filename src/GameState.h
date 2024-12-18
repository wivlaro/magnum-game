#pragma once
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include "GameAssets.h"
#include "MagnumGameApp.h"

namespace MagnumGame {
    class ShadowLight;
}

namespace MagnumGame {
    class Player;

    inline static ResourceKey DebugRendererGroup = "DebugRendererGroup";

    class GameState {
    public:
        explicit GameState(const Timeline& timeline, GameAssets& assets);
        ~GameState();

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

        void drawShadowBuffer();

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

        Containers::Array<Containers::Pointer<GL::Mesh>> _levelMeshes{};
        Containers::Array<Containers::Pointer<btConvexHullShape>> _levelShapes{};
        Containers::Array<GL::Texture2D> _levelTextures{};
        Containers::Array<MaterialAsset> _levelMaterials{};
        std::map<GL::Mesh*, btConvexHullShape*> _meshToShapeMap{};

        DebugTools::ResourceManager _debugResourceManager{};

        SceneGraph::DrawableGroup3D _debugDrawables{};
        SceneGraph::DrawableGroup3D _animatorDrawables{};
        SceneGraph::DrawableGroup3D _opaqueDrawables{};
        SceneGraph::DrawableGroup3D _transparentDrawables{};

        SceneGraph::DrawableGroup3D _shadowCasterDrawables{};

        Scene3D _scene{};
        Containers::Pointer<CameraController> _cameraController;

        Containers::Pointer<Player> _player;

        Containers::Pointer<ShadowLight> _shadowLight;

        bool _isStarted = false;

        btSphereShape _bSphereQueryShape{0.4f};
        btGhostObject _bSphereQueryObject;

        void addDebugDrawable(SceneGraph::AbstractObject3D &playerRigidBody);
    };
}
