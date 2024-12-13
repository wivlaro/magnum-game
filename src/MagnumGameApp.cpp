

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
#include "GameModels.h"

#ifdef BT_USE_DOUBLE_PRECISION
#error sorry, this example does not support Bullet with double precision enabled
#endif

namespace MagnumGame {

    using namespace Math::Literals;

    float MagnumGameApp::cameraCorrectionSpeed = 1.0f;
    float MagnumGameApp::cameraCorrectionAcceleration = 2.0f;
    float MagnumGameApp::cameraCorrectionAngularAcceleration = 2.0f;
    float MagnumGameApp::cameraMinDistance = 20.0f;

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
            if (!tryCreate(conf, glConf))
                create(conf, glConf.setSampleCount(0));
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

        GL::Renderer::Error err;
        while ((err = GL::Renderer::error()) != GL::Renderer::Error::NoError) {
            Error() << "Error at start: " << err;
        }

        //
        _cameraObject = &_scene.addChild<Object3D>(nullptr)
            .translate(Vector3::zAxis(30.0f))
            .rotateX(-90.0_degf);
        _cameraDefaultRotation = Quaternion::fromMatrix(_cameraObject->transformation().rotation());
        _camera = &_cameraObject->addFeature<SceneGraph::Camera3D>();
        _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
            .setProjectionMatrix(Matrix4::perspectiveProjection(_cameraFieldOfView, 1.0f, 0.1f, 100.0f))
            .setViewport(GL::defaultFramebuffer.viewport().size());

        _unlitAlphaShader = Shaders::FlatGL3D{Shaders::FlatGL3D::Configuration{}.setFlags(Shaders::FlatGL3D::Flag::Textured)};

        _texturedShader = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
                                                   .setFlags(Shaders::PhongGL::Flag::DiffuseTexture | Shaders::PhongGL::Flag::ObjectId )};
        _texturedShader.setAmbientColor(0x111111_rgbf)
                .setSpecularColor(0x33000000_rgbaf)
                .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});

        ;
        _animatedTexturedShader = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
            .setJointCount(16, 4)
            .setFlags(Shaders::PhongGL::Flag::DiffuseTexture | Shaders::PhongGL::Flag::ObjectId | Shaders::PhongGL::Flag::DynamicPerVertexJointCount)};
        _animatedTexturedShader.setAmbientColor(0x111111_rgbf)
                .setSpecularColor(0x33000000_rgbaf)
                .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});

        _vertexColorShader = Shaders::VertexColorGL3D{};
        _flatShader = Shaders::FlatGL3D{};


        /* Setup the renderer so we can draw the debug lines on top */
        GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
        GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
        GL::Renderer::enable(GL::Renderer::Feature::PolygonOffsetFill);
        GL::Renderer::setPolygonOffset(2.0f, 0.5f);

        /* Bullet setup */
        _bWorld.setGravity({0.0f, -10.0f, 0.0f});
        _bWorld.setDebugDrawer(&_debugDraw);

        // _scene.addChild<RigidBody>(0.0f, &_bGroundShape, _bWorld);

        setupDebug();
        setup();

        /* Loop at 60 Hz max */
#ifndef CORRADE_TARGET_EMSCRIPTEN
        setSwapInterval(0);
        setMinimalLoopPeriod(8.0_msec);
#endif
        _timeline.start();
    }


    void MagnumGameApp::updateCameraExtents() {
        if (_pointerDrag) {
            _cameraVelocity = {};
            _cameraAngularVelocity = {};
            return;
        }

        // if (auto extents = _gameState->computePlayerExtents()) {
        //
        //     //Move the camera to frame the players
        //
        //     auto avePosition = (extents->first() + extents->second()) * 0.5f;
        //     auto cameraMatrix = _cameraObject->transformation();
        //     auto cameraPosition = cameraMatrix.translation();
        //
        //     //Given the camera FoV, calculate theoretical perfect distance to frame the players
        //     auto targetCameraDistance = Math::max(cameraMinDistance, (extents->second() - extents->first()).length() * 0.5f / Math::tan(_cameraFieldOfView / 2.0f));
        //     auto targetCameraPosition = avePosition + Vector3::yAxis(targetCameraDistance);
        //
        //     auto targetCameraVelocity = (targetCameraPosition - cameraPosition) * cameraCorrectionSpeed;
        //     auto targetCameraRotation = _cameraDefaultRotation;
        //     _cameraVelocity += (targetCameraVelocity - _cameraVelocity) * cameraCorrectionAcceleration * _timeline.previousFrameDuration();
        //     auto cameraRotation = Quaternion::fromMatrix(cameraMatrix.rotation()).normalized();
        //
        //     cameraPosition += _cameraVelocity * _timeline.previousFrameDuration();
        //
        //     //Rotation is not part of usual business in this game, so a simple spherical interpolation without fancy timing is ok.
        //     auto newCameraRotation = slerp(cameraRotation, targetCameraRotation, 0.01f);
        //
        //     cameraMatrix = Matrix4::from(newCameraRotation.toMatrix(), cameraPosition);
        //     //Set up the near/far to be appropriate for the distance to the players (not accounting for rotation at all)
        //     _camera->setProjectionMatrix(Matrix4::perspectiveProjection(_cameraFieldOfView, 1.0f, std::max(0.1f, cameraPosition.y() * 0.5f), std::max(10.0f,cameraPosition.y()*1.5f)));
        //     _cameraObject->setTransformation(cameraMatrix);
        // }
    }


    void MagnumGameApp::drawEvent() {

        _framebuffer
            .clearColor(0, Color3{0.125f})
            .clearColor(1, Vector4ui{})
            .clearDepth(1.0f)
            .bind();

        _gameState->update();


        updateCameraExtents();

        _bWorld.stepSimulation(_timeline.previousFrameDuration(), 5);

        _camera->draw(_animatorDrawables);

        _framebuffer.mapForDraw({{Shaders::PhongGL::ColorOutput, GL::Framebuffer::ColorAttachment{0}},
                                {Shaders::PhongGL::ObjectIdOutput, GL::Framebuffer::ColorAttachment{1}}});
        {
            GL::Renderer::enable(GL::Renderer::Feature::Blending);
            GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

            _camera->draw(_drawables);

            _framebuffer.mapForDraw({{Shaders::PhongGL::ColorOutput, GL::Framebuffer::ColorAttachment{0}},
                                    {Shaders::PhongGL::ObjectIdOutput,  GL::Framebuffer::DrawAttachment::None}});
            //Any order drawing is ok, as everything is basically on the same horizontal plane
            _camera->draw(_transparentDrawables);

            _texturedShader.setProjectionMatrix(_camera->projectionMatrix());
        }

        renderDebug();

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

        { GL::Renderer::Error err; while ((err = GL::Renderer::error()) != GL::Renderer::Error::NoError) { Error() << __FILE__ << ":" << __LINE__ << "Error: " << err; } }
    }




}

MAGNUM_APPLICATION_MAIN(MagnumGame::MagnumGameApp)
