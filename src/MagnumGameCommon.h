#pragma once

#include <Magnum/GL/Renderer.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>

#ifndef CORRADE_TARGET_EMSCRIPTEN
#define MAGNUMGAME_SDL
#else
#define MAGNUMGAME_EMSCRIPTEN
#endif

using namespace Magnum;
using namespace Magnum::Math::Literals;

inline void CheckGLError(const char* file, const int line) {
    static int maxLoggable = 100;
    if (maxLoggable < 0) return;
    GL::Renderer::Error err;
    while ((err = GL::Renderer::error()) != GL::Renderer::Error::NoError) {
        Error{} << file << ":" << line << "Error: " << err;
        if (maxLoggable-- == 0) {
            Error{} << "Logging no more errors from CheckGLError";
            break;
        }
    }
}

#define CHECK_GL_ERROR() CheckGLError(__FILE__, __LINE__)

#define DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete; TypeName& operator=(const TypeName&) = delete;


namespace MagnumGame {

    typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
    typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;
}