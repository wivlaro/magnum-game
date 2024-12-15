#pragma once
#include <Magnum/GL/Mesh.h>

#ifndef CORRADE_TARGET_EMSCRIPTEN
#include <Magnum/Platform/Sdl2Application.h>
#else
#include <Magnum/Platform/EmscriptenApplication.h>
#endif

#include <map>
#include <Corrade/Containers/StridedArrayView.h>
#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Timeline.h>
#include <Magnum/BulletIntegration/DebugDraw.h>
#include <Magnum/DebugTools/ResourceManager.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/SceneGraph/FeatureGroup.h>
#include <Magnum/Text/DistanceFieldGlyphCache.h>
#include <Magnum/Shaders/DistanceFieldVectorGL.h>

#include "Animator.h"
#include "RigidBody.h"
#include "CameraController.h"
#include "DebugLines.h"

namespace MagnumGame {
    class UIText;
    class UIScreen;
    class UserInterface;
    class GameState;
    class Tweakables;
    class Player;
    class GameAssets;
    class Animator;
    struct AnimatorAsset;

    class MagnumGameApp : public Platform::Application {
    public:

        explicit MagnumGameApp(const Arguments &arguments);

        static Containers::Optional<Containers::String> findDirectory(Containers::StringView dirName);

        enum ControllerKeys {
            KEY_NONE = 0,
            KEY_FORWARD = 1,
            KEY_BACKWARD = 2,
            KEY_LEFT = 4,
            KEY_RIGHT = 8,
        };

    private:
        void drawEvent() override;

        void toPauseScreen();

        void keyPressEvent(KeyEvent &event) override;
        void keyReleaseEvent(KeyEvent &event) override;

        void pointerPressEvent(PointerEvent &event) override;

        Vector2 getUIPosition(Vector2 mousePosition) const;

        void pointerReleaseEvent(PointerEvent &event) override;
        void pointerMoveEvent(PointerMoveEvent &event) override;
        void scrollEvent(ScrollEvent &event) override;

#ifdef MAGNUM_SDL2APPLICATION_MAIN
        void anyEvent(SDL_Event &event) override;
#endif

        bool isPlaying();

        GL::Framebuffer _framebuffer{NoCreate};
        GL::Renderbuffer _color{NoCreate}, _objectId{NoCreate}, _depth{NoCreate};

        PluginManager::Manager<Text::AbstractFont> _fontManager;

        Containers::Pointer<UserInterface> _ui;
        UIScreen* _currentScreen{};

        Containers::Pointer<UIScreen> _hudScreen{};
        UIText* _statusText{};

        Containers::Pointer<UIScreen> _menuScreen{};

        Containers::Pointer<UIScreen> _debugScreen{};
        UIText* _debugText{};

        Containers::Pointer<GameAssets> _assets;
        Containers::Pointer<GameState> _gameState;

        Timeline _timeline;

        bool _pointerDrag;
        bool _drawDebug{false};

        Containers::Pointer<DebugLines> _debugLines;
        Containers::Pointer<Tweakables> _tweakables;

        int _controllerKeysHeld;
        std::unordered_map<int,Vector2> _pointerPressLocations{};

        Vector2 getPlayerControlVector() const;

        void updateStatusText();

        void startGame();

        void setupUserInterface();

        void renderTextBuffer(const Matrix3 & matrix3, const Color3& color3, const Color3& outlineColour, GL::Mesh &mesh);

        UnsignedInt pickObjectIdAt(Vector2 eventPosition);

        static ControllerKeys getKeyBit(Key key);
    };

}
