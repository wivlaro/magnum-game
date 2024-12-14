

#include "MagnumGameApp.h"

#include <btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/Path.h>
#include <Magnum/Timeline.h>
#include <Magnum/BulletIntegration/DebugDraw.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Time.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/Transform.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/TextureData.h>
#include <Magnum/Trade/MaterialData.h>
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
#include "GameAssets.h"

#ifdef BT_USE_DOUBLE_PRECISION
#error sorry, this example does not support Bullet with double precision enabled
#endif

namespace MagnumGame {

    using namespace Math::Literals;

    MagnumGameApp::MagnumGameApp(const Arguments &arguments)
    : Platform::Application(arguments, NoCreate)
    {
        /* Try 8x MSAA, fall back to zero samples if not possible. Enable only 2x
           MSAA if we have enough DPI. */
        {
            const Vector2 dpiScaling = this->dpiScaling({});
            Debug{} << "DPI Scaling: " << dpiScaling;
            Configuration conf;
            conf.setTitle("Magnum Game")
                    .setSize({1920, 1080}, dpiScaling);
            GLConfiguration glConf;
#ifndef CORRADE_TARGET_EMSCRIPTEN
            glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
#endif
            if (!tryCreate(conf, glConf)) {
                create(conf, glConf.setSampleCount(0));
            }
        }

        _framebuffer = GL::Framebuffer{GL::defaultFramebuffer.viewport()};
        _color = GL::Renderbuffer{};
        _color.setStorage(GL::RenderbufferFormat::RGBA8, GL::defaultFramebuffer.viewport().size());
        _objectId = GL::Renderbuffer{};
        _objectId.setStorage(GL::RenderbufferFormat::R32UI, GL::defaultFramebuffer.viewport().size());
        _depth = GL::Renderbuffer{};
        _depth.setStorage(GL::RenderbufferFormat::DepthComponent24, GL::defaultFramebuffer.viewport().size());
        _framebuffer.attachRenderbuffer(GL::Framebuffer::ColorAttachment{0}, _color)
                   .attachRenderbuffer(GL::Framebuffer::ColorAttachment{1}, _objectId)
                   .attachRenderbuffer(GL::Framebuffer::BufferAttachment::Depth, _depth)
                   .mapForDraw({{Shaders::PhongGL::ColorOutput, GL::Framebuffer::ColorAttachment{0}},
                                {Shaders::PhongGL::ObjectIdOutput, GL::Framebuffer::ColorAttachment{1}}});

        CHECK_GL_ERROR(__FILE__,__LINE__);

        GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
        GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
        GL::Renderer::enable(GL::Renderer::Feature::PolygonOffsetFill);
        GL::Renderer::setPolygonOffset(2.0f, 0.5f);

        _tweakables.emplace();

        PluginManager::Manager<Trade::AbstractImporter> manager;

        setupTextRenderer();

        auto gltfImporter = manager.loadAndInstantiate("GltfImporter");
        assert(gltfImporter);
        _assets.emplace(*gltfImporter);

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
    }

    void MagnumGameApp::drawEvent() {

        _framebuffer
            .clearColor(0, Color3{0.125f})
            .clearColor(1, Vector4ui{})
            .clearDepth(1.0f)
            .bind();

        _gameState->setControl(getPlayerControlVector());

        _gameState->update();

        //Object picking support
        _framebuffer.mapForDraw({{Shaders::PhongGL::ColorOutput, GL::Framebuffer::ColorAttachment{0}},
                                {Shaders::PhongGL::ObjectIdOutput, GL::Framebuffer::ColorAttachment{1}}});
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

        _gameState->drawOpaque();

        _framebuffer.mapForDraw({{Shaders::PhongGL::ColorOutput, GL::Framebuffer::ColorAttachment{0}},
                                {Shaders::PhongGL::ObjectIdOutput,  GL::Framebuffer::DrawAttachment::None}});

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
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

        renderDebugText();
        renderGameStatusText();

        GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

        GL::AbstractFramebuffer::blit(_framebuffer, GL::defaultFramebuffer, _framebuffer.viewport(), GL::FramebufferBlit::Color);
        swapBuffers();
        _timeline.nextFrame();
        redraw();

        CHECK_GL_ERROR(__FILE__, __LINE__);
    }


}

MAGNUM_APPLICATION_MAIN(MagnumGame::MagnumGameApp)
