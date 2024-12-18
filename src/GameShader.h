#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Matrix3.h>
#include <Magnum/Math/Matrix4.h>
#include <string>
#include <Corrade/Containers/ArrayView.h>
#include <Magnum/Shaders/GenericGL.h>

namespace MagnumGame {

using namespace Magnum;

class GameShader : public GL::AbstractShaderProgram
{
public:

	typedef Shaders::GenericGL3D::Position Position;
	typedef Shaders::GenericGL3D::TextureCoordinates TextureCoordinates;
	typedef Shaders::GenericGL3D::Normal Normal;
	typedef Shaders::GenericGL3D::JointIds JointIds;
	typedef Shaders::GenericGL3D::Weights Weights;

    explicit GameShader(const std::string& vertFilename, const std::string& fragFilename, int maxAnimationBones);

	~GameShader() override = default;

	void addDefine(const std::string& name, const std::string& value);

	enum: Int {
		DiffuseTextureLayer = 0,
		ShadowmapTextureLayer = 1
	};

	GameShader& setAmbientColor(const Vector3& color) {
		setUniform(ambientColorUniform, color);
		return *this;
	}

	GameShader& setTransformationMatrix(const Matrix4& matrix) {
		setUniform(transformationMatrixUniform, matrix);
		return *this;
	}
	GameShader& setNormalMatrix(const Matrix3x3& matrix) {
		setUniform(normalMatrixUniform, matrix);
		return *this;
	}
	GameShader& setProjectionMatrix(const Matrix4& matrix) {
		setUniform(projectionMatrixUniform, matrix);
		return *this;
	}

	GameShader& setModelMatrix(const Matrix4& matrix) {
		setUniform(modelMatrixUniform, matrix);
		return *this;
	}

	GameShader& setShadowCutPlanes(const Corrade::Containers::ArrayView<const float>& shadowCutPlanes) {
		setUniform(shadowCutPlanesUniform, shadowCutPlanes);
		return *this;
	}

	GameShader& setShadowmapMatrices(const Corrade::Containers::ArrayView<Matrix4>& matrices) {
		setUniform(shadowmapMatrixUniform, matrices);
		return *this;
	}

	GameShader& setShadowmapMatrix(const Matrix4& matrix) {
		setUniform(shadowmapMatrixUniform, matrix);
		return *this;
	}

	GameShader& setLightVector(const Vector3& f) {
		setUniform(lightVectorUniform, f);
		return *this;
	}

	GameShader& setLightColor(const Vector3& f) {
		setUniform(lightColorUniform, f);
		return *this;
	}

	GameShader& setSpecularColor(const Vector3& f) {
		setUniform(specularColorUniform, f);
		return *this;
	}


	GameShader& setPerVertexJointCount(int jointCount) {
		setUniform(perVertexJointCountUniform, jointCount);
		return *this;
	}

	GameShader& setJointMatrices(Containers::ArrayView<const Matrix4> array) {
		setUniform(jointMatricesUniform, array);
		return *this;
	}

	GameShader& setShadowmapTexture(GL::Texture2DArray& texture);
	GameShader& setDiffuseTexture(GL::Texture2D& texture);
	
private:
	Int transformationMatrixUniform,
		projectionMatrixUniform,
		normalMatrixUniform,
		modelMatrixUniform,
		shadowmapMatrixUniform,
		shadowCutPlanesUniform,
		specularColorUniform,
		lightVectorUniform,
		lightColorUniform,
		shininessUniform,
		ambientColorUniform,
		perVertexJointCountUniform,
		jointMatricesUniform;

	std::string preamble;


};

}
