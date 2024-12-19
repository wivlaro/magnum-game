#include "ShadowCasterDrawable.h"
#include "Magnum/SceneGraph/Camera.h"
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include "ShadowCasterShader.h"

namespace MagnumGame {
    ShadowCasterDrawable::ShadowCasterDrawable(Object3D &parent, ShadowCasterShader &shader,
                                               SceneGraph::DrawableGroup3D& drawables)
        : SceneGraph::Drawable3D(parent, &drawables)
          , _shader(shader) {
    }


    void ShadowCasterDrawable::draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) {
        _shader.setTransformationMatrix(camera.projectionMatrix() * transformationMatrix);
        CHECK_GL_ERROR();
        if (_skinMeshDrawable.boneMatrices != nullptr) {
            _shader.setPerVertexJointCount(_skinMeshDrawable.perVertexJointCount);
            CHECK_GL_ERROR();
            _shader.setJointMatrices(*_skinMeshDrawable.boneMatrices);
            CHECK_GL_ERROR();
        } else {
            _shader.setPerVertexJointCount(0);
            CHECK_GL_ERROR();
        }
        _shader.draw(*mesh);
        CHECK_GL_ERROR();
    }
}
