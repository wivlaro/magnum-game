#pragma once

#include <Corrade/Containers/ArrayView.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/GenericGL.h>

#include "MagnumGameCommon.h"

namespace MagnumGame {

class ShadowCasterShader : public GL::AbstractShaderProgram {

public:
    typedef Shaders::GenericGL3D::Position Position;

    explicit ShadowCasterShader(const Containers::StringView& vertFilename, const Containers::StringView& fragFilename, int maxAnimationBones);

    auto& setTransformationMatrix(const Matrix4& matrix) {
        setUniform(transformationMatrixUniform, matrix);
        return *this;
    }

    auto& setPerVertexJointCount(UnsignedInt jointCount) {
        setUniform(perVertexJointCountUniform, jointCount);
        return *this;
    }

    auto& setJointMatrices(Containers::ArrayView<const Matrix4> array) {
        setUniform(jointMatricesUniform, array);
        return *this;
    }

private:
    Int transformationMatrixUniform,
        perVertexJointCountUniform,
        jointMatricesUniform;
};

}
