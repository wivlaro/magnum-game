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

GameShader::GameShader(const std::string& vertFilename, const std::string& fragFilename)
{
	CHECK_GL_ERROR(__FILE__, __LINE__);
	addDefine("NUM_SHADOW_MAP_LEVELS","4");
	addDefine("MAX_JOINT_COUNT","16");

	CHECK_GL_ERROR(__FILE__, __LINE__);
	using namespace Magnum;
	const Version version = Context::current().version();
    // const Version version = Context::current().supportedVersion({Version::GL410, Version::GL400, Version::GL330, Version::GL320, Version::GL310, Version::GL300, Version::GL210});
	CHECK_GL_ERROR(__FILE__, __LINE__);

	// Load shader sources
    Shader vert(version, Shader::Type::Vertex);
    Shader frag(version, Shader::Type::Fragment);
	CHECK_GL_ERROR(__FILE__, __LINE__);
	vert.addSource(preamble);
	frag.addSource(preamble);
	CHECK_GL_ERROR(__FILE__, __LINE__);
	preamble = "";
	vert.addFile(vertFilename);
    frag.addFile(fragFilename);
	CHECK_GL_ERROR(__FILE__, __LINE__);
	Debug{} << "Compiling shader " << vertFilename << " " << fragFilename << version;
#ifndef MAGNUM_TARGET_WEBGL
	setLabel(vertFilename + " & " + fragFilename);
#endif
	CHECK_GL_ERROR(__FILE__, __LINE__);
	std::cout.flush();
	vert.submitCompile();
	frag.submitCompile();
	if (!vert.checkCompile() || !frag.checkCompile()) {
		throw std::runtime_error("Failed to compile " + vertFilename + " & " + fragFilename);
	}
	CHECK_GL_ERROR(__FILE__, __LINE__);

	bindAttributeLocation(Position::Location, "position");
	bindAttributeLocation(Normal::Location, "normal");
	bindAttributeLocation(TextureCoordinates::Location, "textureCoordinates");
	bindAttributeLocation(JointIds::Location, "jointIds");
	bindAttributeLocation(Weights::Location, "weights");

    // Attach the shaders
    attachShader(vert);
    attachShader(frag);
	CHECK_GL_ERROR(__FILE__, __LINE__);

    // Link the program together
	std::cout.flush();
	if (!link()) {
		throw std::runtime_error("Failed to link " + vertFilename + " & " + fragFilename);
	}
	CHECK_GL_ERROR(__FILE__, __LINE__);

	transformationMatrixUniform = uniformLocation("transformationMatrix");
	projectionMatrixUniform = uniformLocation("projectionMatrix");
	normalMatrixUniform = uniformLocation("normalMatrix");
	modelMatrixUniform = uniformLocation("modelMatrix");
	shadowmapMatrixUniform = uniformLocation("shadowmapMatrix");
	shadowCutPlanesUniform = uniformLocation("shadowDepthSplits");
	lightVectorUniform = uniformLocation("light");
	shininessUniform = uniformLocation("shininess");
	ambientColorUniform = uniformLocation("ambientColor");
	jointMatricesUniform = uniformLocation("jointMatrices");
	perVertexJointCountUniform = uniformLocation("perVertexJointCount");

	specularColorUniform = uniformLocation("specularColor");

	setUniform(uniformLocation("diffuseTexture"), DiffuseTextureLayer);
	setUniform(uniformLocation("shadowmapTexture"), ShadowmapTextureLayer);

	std::cout << "\nSHADER " << vertFilename << " & " << fragFilename << "\nAttribute locations:\n\tposition=" << Position::Location << "\n\tnormal=" << Normal::Location << "\n\ttexcoords=" << TextureCoordinates::Location << std::endl;
	std::cout << "bound: position = " << glGetAttribLocation(id(), "position") << std::endl;
	std::cout << "bound: textureCoords = " << glGetAttribLocation(id(), "textureCoordinates") << std::endl;
	std::cout << "bound: normal = " << glGetAttribLocation(id(), "normal") << std::endl;
	std::cout << "bound: particlePosition = " << glGetAttribLocation(id(), "particlePosition") << std::endl;
	std::cout << "bound: shadowmapTexture = " << uniformLocation("shadowmapTexture") << std::endl;
	std::cout << "bound: diffuseTexture = " << uniformLocation("diffuseTexture") << std::endl;
	std::cout << "bound: lightmapTexture = " << uniformLocation("lightmapTexture") << std::endl;
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
