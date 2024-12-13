//
// Created by Bill Robinson on 17/07/2024.
//

#pragma once
#include <Magnum/GL/AbstractShaderProgram.h>
#include "NamePickerCommon.h"

namespace MagnumGame {

    using namespace Magnum;

    class UnlitAlphaShader: public GL::AbstractShaderProgram {
    public:
        typedef GL::Attribute<0, Vector2> Position;
        typedef GL::Attribute<1, Vector2> TextureCoordinates;

        explicit UnlitAlphaShader(NoCreateT) : AbstractShaderProgram{NoCreate} {}
        explicit UnlitAlphaShader();

        UnlitAlphaShader& setProjectionMatrix(const Magnum::Matrix4& matrix);

        UnlitAlphaShader& setTransformationMatrix(const Magnum::Matrix4& matrix);

        UnlitAlphaShader& setColor(const Color4& color);

        UnlitAlphaShader& bindTexture(GL::Texture2D& texture);

    private:
        enum: Int { TextureUnit = 0 };

        Int _colorUniform;
        Int _projectionMatrixUniform;
        Int _transformationMatrixUniform;
    };

} // NamePicker

