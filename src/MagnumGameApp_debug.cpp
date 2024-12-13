#include <Magnum/DebugTools/ObjectRenderer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Trade/MeshData.h>

#include "DebugLines.h"
#include "GameState.h"
#include "MagnumGameApp.h"
#include "Player.h"
#include "Tweakables.h"

namespace MagnumGame {

    static ResourceKey DebugRendererGroup = "DebugRendererGroup";

    void MagnumGameApp::setupDebug() {


        _debugLinesBuffer = GL::Buffer{GL::Buffer::TargetHint::Array};
        _debugLinesBuffer.setData(DebugLines::LineData, GL::BufferUsage::DynamicDraw);

        _debugLinesMesh = GL::Mesh{GL::MeshPrimitive::Lines};
        _debugLinesMesh.setCount(static_cast<int>(DebugLines::LineData.size()));

        _debugLinesMesh.addVertexBuffer(_debugLinesBuffer, 0, 0, Shaders::VertexColorGL3D::Position{}, Shaders::VertexColorGL3D::Color4{});

        _debugDraw = BulletIntegration::DebugDraw{};
        _debugDraw.setMode(BulletIntegration::DebugDraw::Mode::DrawWireframe);

        _tweakables = std::make_unique<Tweakables>();

        _debugResourceManager.set(DebugRendererGroup, DebugTools::ObjectRendererOptions{}.setSize(3.f));
    }

    void MagnumGameApp::addDebugDrawable(RigidBody &playerRigidBody) {
        //This is a feature added to playerRigidBody - it is deleted with that. Not nice to allocate in app code and delete in library code.
        new DebugTools::ObjectRenderer3D{
            _debugResourceManager, playerRigidBody, DebugRendererGroup, &_debugDrawables
        };
    }

    void MagnumGameApp::renderDebug() {
        auto transformationProjectionMatrix = _camera->projectionMatrix() * _camera->cameraMatrix();
        if (_drawDebug) {
            GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::LessOrEqual);

            _debugDraw.setTransformationProjectionMatrix(transformationProjectionMatrix);
            _bWorld.debugDrawWorld();

            GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Less);
        }

        if (_tweakables && _tweakables->hasActiveDebugMode()) {
            _debugLinesBuffer.setData(DebugLines::LineData, GL::BufferUsage::DynamicDraw);
            _debugLinesMesh.setCount(DebugLines::LineData.size());
            _vertexColorShader.setTransformationProjectionMatrix(transformationProjectionMatrix);
            _vertexColorShader.draw(_debugLinesMesh);
        }
        DebugLines::Clear();
    }


    float MagnumGameApp::getTweakAmount(const KeyEvent &event, float value) {
        auto powerAdjust = -1;
        if (event.modifiers() & Modifier::Shift) {
            powerAdjust -= 1;
        }
        else if (event.modifiers() & (Modifier::Ctrl|Modifier::Alt)) {
            powerAdjust += 1;
        }
        //Compute the nearest sensible power of 10 to for adjusting a value
        return pow(10, round(log10(abs(value))) + powerAdjust);
    }
}
