#pragma once
#include <Magnum/GL/Mesh.h>

#ifndef CORRADE_TARGET_EMSCRIPTEN
#include <Magnum/Platform/Sdl2Application.h>
#else
#include <Magnum/Platform/EmscriptenApplication.h>
#endif

#include <map>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <Corrade/Containers/StridedArrayView.h>
#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Timeline.h>
#include <Magnum/BulletIntegration/DebugDraw.h>
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/DebugTools/ResourceManager.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/SceneGraph/FeatureGroup.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Text/DistanceFieldGlyphCache.h>
#include <Magnum/Shaders/DistanceFieldVectorGL.h>
#include <Magnum/SceneGraph/SceneGraph.h>
#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Trade/ImageData.h>

#include "Animator.h"
#include "MagnumGameCommon.h"
#include "RigidBody.h"

namespace MagnumGame {
    class GameState;
    class Tweakables;
    class Player;
    class GameModels;
    class Animator;

    class MagnumGameApp : public Platform::Application {
    public:

#ifdef CORRADE_TARGET_EMSCRIPTEN
        typedef KeyEvent::Key Key;
        typedef KeyEvent::Modifier Modifier;
#endif

        void setupDebug();

        explicit MagnumGameApp(const Arguments &arguments);

        static float cameraCorrectionSpeed;
        static float cameraCorrectionAcceleration;
        static float cameraCorrectionAngularAcceleration;
        static float cameraMinDistance;

        ///In bullet physics, we use UserIndex1 to say what kind of data is stored in the next index - we only have player indices for now
        static constexpr int World_UserIndex1_Player = 1;


        static Containers::Optional<Containers::String> findDirectory(Containers::StringView dirName);

    private:
        void drawEvent() override;

        void keyPressEvent(KeyEvent &event) override;
        void mousePressEvent(MouseEvent &event) override;
        void mouseReleaseEvent(MouseEvent &event) override;
        void mouseMoveEvent(MouseMoveEvent &event) override;

        GL::Framebuffer _framebuffer{NoCreate};
        GL::Renderbuffer _color{NoCreate}, _objectId{NoCreate}, _depth{NoCreate};

        DebugTools::ResourceManager _debugResourceManager;
        SceneGraph::DrawableGroup3D _debugDrawables;

        GL::Mesh _playerMesh{NoCreate};

        GL::Mesh _debugLinesMesh{NoCreate};
        GL::Buffer _debugLinesBuffer{NoCreate};
        GL::Mesh _arenaDebugMesh{NoCreate};

        std::vector<Containers::Pointer<GL::Mesh>> _levelMeshes{};
        std::vector<Containers::Pointer<btConvexHullShape>> _levelShapes{};
        std::vector<GL::Texture2D> _levelTextures{};
        std::map<GL::Mesh*, btConvexHullShape*> _meshToShapeMap{};

        std::shared_ptr<GL::Texture2D> _thrustTexture, _laserTexture, _explosionTexture, _groundTexture;

        Matrix4x4 _laserTransform, _explosionTransform, _thrustTransform;

        PluginManager::Manager<Text::AbstractFont> _fontManager;
        Containers::Pointer<Text::AbstractFont> _font;
        Shaders::DistanceFieldVectorGL2D _textShader{NoCreate};
        Text::DistanceFieldGlyphCacheGL _fontGlyphCache{NoCreate};
        Containers::Pointer<Text::Renderer2D> _debugTextRenderer;
        GL::Buffer _textVertexBuffer{NoCreate}, _textIndexBuffer{NoCreate};
        GL::Mesh _textMesh{NoCreate};

        std::unique_ptr<GameModels> _models;

        Shaders::PhongGL _texturedShader{NoCreate};
        Shaders::PhongGL _animatedTexturedShader{NoCreate};
        Shaders::FlatGL3D _unlitAlphaShader{NoCreate};
        BulletIntegration::DebugDraw _debugDraw{NoCreate};
        Shaders::VertexColorGL3D _vertexColorShader{NoCreate};
        Shaders::FlatGL3D _flatShader{NoCreate};

        std::unique_ptr<GameState> _gameState;

        btDbvtBroadphase _bBroadphase;
        btDefaultCollisionConfiguration _bCollisionConfig;
        btCollisionDispatcher _bDispatcher{&_bCollisionConfig};
        btSequentialImpulseConstraintSolver _bSolver;

        /* The world has to live longer than the scene because RigidBody
           instances have to remove themselves from it on destruction */
        btDiscreteDynamicsWorld _bWorld{&_bDispatcher, &_bBroadphase, &_bSolver, &_bCollisionConfig};

        Scene3D _scene;
        SceneGraph::Camera3D *_camera;
        Deg _cameraFieldOfView{35.0_degf};
        Vector3 _cameraVelocity;
        Quaternion _cameraAngularVelocity;
        SceneGraph::DrawableGroup3D _drawables{};
        SceneGraph::DrawableGroup3D _animatorDrawables{};
        SceneGraph::DrawableGroup3D _transparentDrawables{};
        Timeline _timeline;

        Object3D *_cameraObject;

        bool _pointerDrag;
        Quaternion _cameraDefaultRotation;

        bool _drawDebug{false};

        std::unique_ptr<Tweakables> _tweakables;
        Object3D* _player;
        Animator* _playerAnimator;

        Containers::Pair<Object3D*, Animator*> loadAnimatedModel(Trade::AbstractImporter &pointer, Containers::StringView fileName);

        void setup();

        void updateCameraExtents();

        void renderDebug();

        void renderDebugText();

        void renderGameStatusText();

        void loadLevel(Trade::AbstractImporter &importer);

        void setupTextRenderer();

        void renderTextBuffer(const Matrix3 & matrix3, const Color3& color3, const Color3& outline_colour, GL::Mesh &mesh);

        static float getTweakAmount(const KeyEvent &event, float value);

        UnsignedInt pickObjectIdAt(Vector2i eventPosition);

        void addDebugDrawable(RigidBody &playerRigidBody);

        ///For object picking, where do the players start
        static constexpr int PlayerIdOffset = 16;
    };
}
