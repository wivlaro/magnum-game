//
// Created by Bill Robinson on 17/07/2024.
//

#include "UnlitAlphaShader.h"
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Color.h>
#include <Corrade/Utility/Resource.h>

namespace MagnumGame {

    UnlitAlphaShader::UnlitAlphaShader()
    {
        const Utility::Resource rs{"namepicker-data"};

        GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
        GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};

        vert.addSource(rs.getString("UnlitAlpha.vert"));
        frag.addSource(rs.getString("UnlitAlpha.frag"));

        CORRADE_INTERNAL_ASSERT_OUTPUT(vert.compile() && frag.compile());

        attachShaders({vert, frag});

        CORRADE_INTERNAL_ASSERT_OUTPUT(link());

        _colorUniform = uniformLocation("color");
        _projectionMatrixUniform = uniformLocation("projectionMatrix");
        _transformationMatrixUniform = uniformLocation("transformationMatrix");
        setUniform(uniformLocation("textureData"), TextureUnit);
    }

    UnlitAlphaShader & UnlitAlphaShader::setProjectionMatrix(const Matrix4 &matrix) {
        setUniform(_projectionMatrixUniform, matrix);
        return *this;
    }


    UnlitAlphaShader & UnlitAlphaShader::setTransformationMatrix(const Matrix4 &matrix) {
        setUniform(_transformationMatrixUniform, matrix);
        return *this;
    }

    UnlitAlphaShader & UnlitAlphaShader::setColor(const Color4 &color) {
        setUniform(_colorUniform, color);
        return *this;
    }

    UnlitAlphaShader & UnlitAlphaShader::bindTexture(GL::Texture2D &texture) {
        texture.bind(TextureUnit);
        return *this;
    }
} // NamePicker