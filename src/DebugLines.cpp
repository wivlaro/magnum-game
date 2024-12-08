//
// Created by Bill Robinson on 17/07/2024.
//

#include "DebugLines.h"

#include <Corrade/Containers/GrowableArray.h>

namespace MagnumGame {
    Containers::Array<DebugLines::DebugLinePoint> DebugLines::LineData{};

    void DebugLines::Add(const Vector3 &p1, const Vector3 &p2, const Color4 &color) {
        arrayAppend(LineData, DebugLinePoint{p1, color});
        arrayAppend(LineData, DebugLinePoint{p2, color});
    }

    void DebugLines::Add(const btVector3 &p1, const btVector3 &p2, const Color4 &color) {
        arrayAppend(LineData, DebugLinePoint{Vector3(p1.x(), p1.y(), p1.z()), color});
        arrayAppend(LineData, DebugLinePoint{Vector3(p2.x(), p2.y(), p2.z()), color});
    }

    void DebugLines::Clear() {
        arrayResize(LineData, 0);
    }

}
