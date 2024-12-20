#include "GameShader.h"

#include <Magnum/GL/Version.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Context.h>
#include <Corrade/Utility/Assert.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureArray.h>
#include <Magnum/GL/Attribute.h>
#include <Magnum/GL/Renderer.h>
#include <Corrade/Utility/DebugStl.h>
#include <Corrade/Containers/Reference.h>
#include <iostream>

#include "MagnumGameCommon.h"

namespace MagnumGame {

	using namespace Magnum::GL;

GameShader::GameShader(const std::string& vertFilename, const std::string& fragFilename, int maxAnimationBones, int shadowMapLevels, bool shadowPcf)
{
	CHECK_GL_ERROR();
	if (shadowMapLevels > 0) {
		addDefine("ENABLE_SHADOWMAP_LEVELS",std::to_string(shadowMapLevels));
	}
	if (maxAnimationBones > 0) {
		addDefine("ENABLE_MAX_ANIMATION_BONES",std::to_string(maxAnimationBones));
	}
	if (shadowPcf) {
		addDefine("SHADOWMAP_PCF", "1");
	}

	CHECK_GL_ERROR();
	using namespace Magnum;
	const Version version = Context::current().version();
    // const Version version = Context::current().supportedVersion({Version::GL410, Version::GL400, Version::GL330, Version::GL320, Version::GL310, Version::GL300, Version::GL210});
	CHECK_GL_ERROR();

	// Load shader sources
    Shader vert(version, Shader::Type::Vertex);
    Shader frag(version, Shader::Type::Fragment);
	CHECK_GL_ERROR();
	vert.addSource(preamble);
	frag.addSource(preamble);
	CHECK_GL_ERROR();
	preamble = "";
	vert.addFile(vertFilename);
    frag.addFile(fragFilename);
	CHECK_GL_ERROR();
	Debug{} << "Compiling shader " << vertFilename << " " << fragFilename << version;
#ifndef MAGNUM_TARGET_WEBGL
	setLabel(vertFilename + " & " + fragFilename);
#endif
	CHECK_GL_ERROR();
	std::cout.flush();
	vert.submitCompile();
	frag.submitCompile();
	if (!vert.checkCompile() || !frag.checkCompile()) {
		throw std::runtime_error("Failed to compile " + vertFilename + " & " + fragFilename);
	}
	CHECK_GL_ERROR();

	bindAttributeLocation(Position::Location, "position");
	bindAttributeLocation(Normal::Location, "normal");
	bindAttributeLocation(TextureCoordinates::Location, "textureCoordinates");
	bindAttributeLocation(JointIds::Location, "jointIds");
	bindAttributeLocation(Weights::Location, "weights");

    // Attach the shaders
    attachShader(vert);
    attachShader(frag);
	CHECK_GL_ERROR();

    // Link the program together
	std::cout.flush();
	if (!link()) {
		throw std::runtime_error("Failed to link " + vertFilename + " & " + fragFilename);
	}
	CHECK_GL_ERROR();

	transformationMatrixUniform = uniformLocation("transformationMatrix");
	projectionMatrixUniform = uniformLocation("projectionMatrix");
	normalMatrixUniform = uniformLocation("normalMatrix");
	modelMatrixUniform = uniformLocation("modelMatrix");
	shadowmapMatrixUniform = uniformLocation("shadowmapMatrix");
	shadowCutPlanesUniform = uniformLocation("shadowDepthSplits");
	lightVectorUniform = uniformLocation("light");
	lightColorUniform = uniformLocation("lightColor");
	shininessUniform = uniformLocation("shininess");
	ambientColorUniform = uniformLocation("ambientColor");
	jointMatricesUniform = uniformLocation("jointMatrices");
	perVertexJointCountUniform = uniformLocation("perVertexJointCount");


	specularColorUniform = uniformLocation("specularColor");

	setUniform(uniformLocation("diffuseTexture"), DiffuseTextureLayer);
	setUniform(uniformLocation("shadowmapTexture"), ShadowmapTextureLayer);

	Debug{} << "\nSHADER " << vertFilename << " & " << fragFilename << "Attributes:"
			<< "position=" << Position::Location << glGetAttribLocation(id(), "position")
			<< "normal=" << Normal::Location << glGetAttribLocation(id(), "normal")
			<< "texcoords=" << TextureCoordinates::Location << glGetAttribLocation(id(), "textureCoordinates")
			<< "Uniforms:"
			<< "shadowmapTexture=" << uniformLocation("shadowmapTexture")
			<< "diffuseTexture=" << uniformLocation("diffuseTexture")
			<< "lightmapTexture=" << uniformLocation("lightmapTexture")
			<< "shininess=" << shininessUniform ;
}

void GameShader::addDefine(const std::string &name, const std::string &value) {
	preamble += "#define " + name + " " + value + "\n";
}



GameShader& GameShader::setDiffuseTexture(Magnum::GL::Texture2D& texture) {
    texture.bind(DiffuseTextureLayer);
    return *this;
}

GameShader& GameShader::setShadowmapTexture(Magnum::GL::Texture2DArray& texture) {
    texture.bind(ShadowmapTextureLayer);
    return *this;
}



}
