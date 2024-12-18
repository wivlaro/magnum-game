#pragma once

#include <Magnum/SceneGraph/MatrixTransformation3D.h>

#define STR(s) #s
#define XSTR(x) STR(x)
#define CHECK_GL_ERROR() { GL::Renderer::Error err; while ((err = GL::Renderer::error()) != GL::Renderer::Error::NoError) { Error() << (__FILE__ ":" STR(__LINE__)) << "Error: " << err; } }

#define DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete; TypeName& operator=(const TypeName&) = delete;

using namespace Magnum;
using namespace Magnum::Math::Literals;

namespace MagnumGame {

    typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
    typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;
}