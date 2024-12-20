
#include "MagnumGameApp.h"

#include <btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/Path.h>
#include <Magnum/Timeline.h>
#include <Magnum/BulletIntegration/DebugDraw.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Time.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/Transform.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/TextureData.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/TextureTools/Atlas.h>
#include <Magnum/ImageView.h>
#include <Magnum/Text/Renderer.h>
#include <Magnum/DebugTools/ObjectRenderer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/PixelFormat.h>
#include "MagnumGameCommon.h"
#include "RigidBody.h"
#include "Player.h"
#include "Tweakables.h"
#include <sstream>

#include "DebugLines.h"
#include "GameState.h"
#include "UserInterface.h"

#ifdef MAGNUMGAME_SDL
#include <SDL2/SDL.h>
#include "SdlGameController.h"
#endif

#ifdef BT_USE_DOUBLE_PRECISION
#error sorry, this example does not support Bullet with double precision enabled
#endif

namespace MagnumGame {

    using namespace Math::Literals;

    MagnumGameApp::MagnumGameApp(const Arguments &arguments)
    : Platform::Application(arguments, NoCreate)
    , _pointerDrag{}
    , _controllerKeysHeld{}
    {
        /* Try 8x MSAA, fall back to zero samples if not possible. Enable only 2x
           MSAA if we have enough DPI. */
        {
            const Vector2 dpiScaling = this->dpiScaling({});
            Debug{} << "DPI Scaling: " << dpiScaling;
            Configuration conf;
            conf.setTitle("Magnum Game").setSize({1920, 1080}, dpiScaling);
            GLConfiguration glConf;
#ifndef CORRADE_TARGET_EMSCRIPTEN
            glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
#endif
            if (!tryCreate(conf, glConf)) {
                create(conf, glConf.setSampleCount(0));
            }
        }

        CHECK_GL_ERROR();

        GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
        GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
        GL::Renderer::enable(GL::Renderer::Feature::PolygonOffsetFill);
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
        GL::Renderer::setPolygonOffset(2.0f, 0.5f);

        _tweakables.emplace();

        PluginManager::Manager<Trade::AbstractImporter> manager;

        auto gltfImporter = manager.loadAndInstantiate("GltfImporter");
        assert(gltfImporter);
        _assets.emplace(*gltfImporter);

        setupUserInterface();

        _gameState.emplace(_timeline, *_assets);
        _gameState->loadLevel(*gltfImporter);
        _gameState->setupPlayer();

        _tweakables->addDebugMode("Friction", 0, {
                                      {
                                          "Friction", [&]() {
                                              return _gameState->getPlayer()->getBody()->rigidBody().getFriction();
                                          },
                                          [&](float value) {
                                              _gameState->getPlayer()->getBody()->rigidBody().setFriction(value);
                                          }
                                      },
                                      {
                                          "Rolling Friction", [&]() {
                                              return _gameState->getPlayer()->getBody()->rigidBody().
                                                      getRollingFriction();
                                          },
                                          [&](float value) {
                                              _gameState->getPlayer()->getBody()->rigidBody().setRollingFriction(value);
                                          }
                                      }
                                  });

#ifndef CORRADE_TARGET_EMSCRIPTEN
        setSwapInterval(0);
        setMinimalLoopPeriod(8.0_msec);
#endif
        _timeline.start();

#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_Init(SDL_INIT_GAMECONTROLLER);
        _gameController.emplace(_gameState);
#endif
    }

    bool MagnumGameApp::isPlaying() {
        return _hudScreen && _currentScreen == _hudScreen.get();
    }

    void MagnumGameApp::startGame() {
        _currentScreen = _hudScreen.get();
    }

    void MagnumGameApp::toPauseScreen() {
        _currentScreen = _menuScreen.get();
    }

    void MagnumGameApp::drawEvent() {

        GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

        if (isPlaying()) {

#ifdef MAGNUMGAME_SDL
            auto cameraControl = _gameController->getCameraDirectionalControlVector();
            if (!cameraControl.isZero()) {
                _gameState->getCamera()->rotateBy(cameraControl.x() * 90.0_degf * _timeline.previousFrameDuration(), cameraControl.y() * -90.0_degf * _timeline.previousFrameDuration());
            }
#endif

            _gameState->setControl(getPlayerControlVector());
            _gameState->update();
            updateStatusText();
        }

        _gameState->drawShadowBuffer();

        _gameState->drawOpaque();

        _gameState->drawTransparent();

        auto transformationProjectionMatrix = _gameState->getCamera()->getTransformationProjectionMatrix();
        if (_drawDebug) {
            GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::LessOrEqual);

            _gameState->renderDebug(transformationProjectionMatrix);

            GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Less);
        }

        if (_debugLines) {
            if (_tweakables && _tweakables->hasActiveDebugMode()) {
                _debugLines->draw(transformationProjectionMatrix);
            }
            _debugLines->clear();
        }

        GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

        if (_currentScreen) {
            _ui->draw(Vector2(windowSize()), *_currentScreen);
        }

        if (_tweakables->currentDebugMode().getModeName()) {
            _debugText->setText(_tweakables->getDebugText());
            _ui->draw(Vector2(windowSize()), *_debugScreen);
        }

        GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

        swapBuffers();
        CHECK_GL_ERROR();
        _timeline.nextFrame();
        redraw();
    }


}

MAGNUM_APPLICATION_MAIN(MagnumGame::MagnumGameApp)
