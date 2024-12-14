//
// Created by Bill Robinson on 17/07/2024.
//

#include "DebugLines.h"

#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/Shaders/VertexColorGL.h>

namespace MagnumGame {
    DebugLines::DebugLines(Shaders::VertexColorGL3D &shader)
        : _debugLinesBuffer{GL::Buffer::TargetHint::Array}
          , _debugLinesMesh{GL::MeshPrimitive::Lines}
          , _shader(shader) {
        _debugLinesBuffer.setData(_lineData, GL::BufferUsage::DynamicDraw);
        _debugLinesMesh.setCount(static_cast<int>(_lineData.size()));

        _debugLinesMesh.addVertexBuffer(_debugLinesBuffer, 0, 0, Shaders::VertexColorGL3D::Position{},
                                        Shaders::VertexColorGL3D::Color4{});
    }

    void DebugLines::add(const Vector3 &p1, const Vector3 &p2, const Color4 &color) {
        arrayAppend(_lineData, DebugLinePoint{p1, color});
        arrayAppend(_lineData, DebugLinePoint{p2, color});
    }

    void DebugLines::add(const btVector3 &p1, const btVector3 &p2, const Color4 &color) {
        arrayAppend(_lineData, DebugLinePoint{Vector3(p1.x(), p1.y(), p1.z()), color});
        arrayAppend(_lineData, DebugLinePoint{Vector3(p2.x(), p2.y(), p2.z()), color});
    }

    void DebugLines::clear() {
        arrayResize(_lineData, 0);
    }

    void DebugLines::draw(const Matrix4 &transformationProjectionMatrix) {
        _debugLinesBuffer.setData(_lineData, GL::BufferUsage::DynamicDraw);
        _debugLinesMesh.setCount(_lineData.size());

        _shader.setTransformationProjectionMatrix(transformationProjectionMatrix);
        _shader.draw(_debugLinesMesh);
    }
}
