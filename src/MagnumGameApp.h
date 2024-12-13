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
#include "AnimatorAsset.h"
#include "MagnumGameCommon.h"
#include "RigidBody.h"
#include "TrackingCamera.h"

namespace MagnumGame {
    class GameState;
    class Tweakables;
    class Player;
    class GameModels;
    class Animator;
    struct AnimatorAsset;

    class MagnumGameApp : public Platform::Application {
    public:

#ifdef CORRADE_TARGET_EMSCRIPTEN
        typedef KeyEvent::Key Key;
        typedef KeyEvent::Modifier Modifier;
#endif

        void setupDebug();

        explicit MagnumGameApp(const Arguments &arguments);

        static Containers::Optional<Containers::String> findDirectory(Containers::StringView dirName);

        enum ControllerKeys {
            KEY_FORWARD = 1,
            KEY_BACKWARD = 2,
            KEY_LEFT = 4,
            KEY_RIGHT = 8,
        };

    private:
        void drawEvent() override;

        void keyPressEvent(KeyEvent &event) override;
        void keyReleaseEvent(KeyEvent &event) override;
        void mousePressEvent(MouseEvent &event) override;
        void mouseReleaseEvent(MouseEvent &event) override;
        void mouseMoveEvent(MouseMoveEvent &event) override;

#ifdef MAGNUM_SDL2APPLICATION_MAIN
        void anyEvent(SDL_Event &event) override;
#endif

        GL::Framebuffer _framebuffer{NoCreate};
        GL::Renderbuffer _color{NoCreate}, _objectId{NoCreate}, _depth{NoCreate};

        DebugTools::ResourceManager _debugResourceManager;
        SceneGraph::DrawableGroup3D _debugDrawables;

        GL::Mesh _debugLinesMesh{NoCreate};
        GL::Buffer _debugLinesBuffer{NoCreate};

        std::vector<Containers::Pointer<GL::Mesh>> _levelMeshes{};
        std::vector<Containers::Pointer<btConvexHullShape>> _levelShapes{};
        Containers::Array<GL::Texture2D> _levelTextures{};
        Containers::Array<MaterialAsset> _levelMaterials{};
        std::map<GL::Mesh*, btConvexHullShape*> _meshToShapeMap{};


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
        SceneGraph::DrawableGroup3D _drawables{};
        SceneGraph::DrawableGroup3D _animatorDrawables{};
        SceneGraph::DrawableGroup3D _transparentDrawables{};
        Timeline _timeline;

        Object3D *_cameraObject;
        Containers::Pointer<TrackingCamera> _trackingCamera;

        bool _pointerDrag;
        Quaternion _cameraDefaultRotation;

        bool _drawDebug{false};

        std::unique_ptr<AnimatorAsset> _playerAsset{};
        std::unique_ptr<Tweakables> _tweakables;
        RigidBody* _playerBody{};
        Animator* _playerAnimator{};

        int controllerKeysHeld;

        std::unique_ptr<AnimatorAsset> loadAnimatedModel(Trade::AbstractImporter &pointer,
                                                         Containers::StringView fileName);

        void createPlayer();

        void setup();


        void renderDebug();

        void renderDebugText();

        Vector2 getPlayerControlVector();

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
