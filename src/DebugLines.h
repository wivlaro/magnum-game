//
// Created by Bill Robinson on 17/07/2024.
//

#pragma once
#include <LinearMath/btVector3.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Vector3.h>

namespace MagnumGame {

    using namespace Magnum;


    /**
     * @brief Utility to draw lines each frame on the screen in world space
     */
    class DebugLines {
    public:

        struct DebugLinePoint {
            Vector3 position;
            Color4 color;
        };
        static Containers::Array<DebugLinePoint> LineData;
        static void Add(const Vector3 &p1, const Vector3 &p2, const Color4 &color);
        static void Add(const btVector3& p1, const btVector3& p2, const Color4& color);
        static void Clear();
    };

}
