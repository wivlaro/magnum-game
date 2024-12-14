//
// Created by Bill Robinson on 17/07/2024.
//

#pragma once
#include <Corrade/Containers/Array.h>
#include <LinearMath/btVector3.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Shaders/VertexColorGL.h>

namespace MagnumGame {

    using namespace Magnum;


    /**
     * @brief Utility to draw lines each frame on the screen in world space
     */
    class DebugLines {
    public:
        explicit DebugLines(Shaders::VertexColorGL3D &shader);

        void add(const Vector3 &p1, const Vector3 &p2, const Color4 &color);
        void add(const btVector3& p1, const btVector3& p2, const Color4& color);
        void clear();

        void draw(const Matrix4 &transformationProjectionMatrix);

    private:

        struct DebugLinePoint {
            Vector3 position;
            Color4 color;
        };

        Containers::Array<DebugLinePoint> _lineData{};
        GL::Buffer _debugLinesBuffer;
        GL::Mesh _debugLinesMesh;
        Shaders::VertexColorGL3D& _shader;
    };

}
