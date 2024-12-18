//
// Created by Bill Robinson on 18/12/2024.
//

#include "ShadowCasterShader.h"

#include <iostream>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>

#include "MagnumGameCommon.h"

namespace MagnumGame {

	using namespace Magnum::GL;

ShadowCasterShader::ShadowCasterShader(const Containers::StringView &vertFilename, const Containers::StringView &fragFilename, int maxAnimationBones) {

	CHECK_GL_ERROR();

	CHECK_GL_ERROR();
	const Version version = Context::current().version();
    // const Version version = Context::current().supportedVersion({Version::GL410, Version::GL400, Version::GL330, Version::GL320, Version::GL310, Version::GL300, Version::GL210});
	CHECK_GL_ERROR();

	// Load shader sources
    Shader vert(version, Shader::Type::Vertex);
    Shader frag(version, Shader::Type::Fragment);
	CHECK_GL_ERROR();
	if (maxAnimationBones > 0) {
		vert.addSource("#define ENABLE_MAX_ANIMATION_BONES " + std::to_string(maxAnimationBones)+"\n");
	}
	vert.addFile(vertFilename);
    frag.addFile(fragFilename);
	CHECK_GL_ERROR();
	Debug{} << "Compiling shader " << vertFilename << " " << fragFilename << static_cast<int>(version);
#ifndef MAGNUM_TARGET_WEBGL
	setLabel(vertFilename + " & " + fragFilename);
#endif
	CHECK_GL_ERROR();
	vert.submitCompile();
	frag.submitCompile();
	if (!vert.checkCompile() || !frag.checkCompile()) {
		throw std::runtime_error("Failed to compile " + vertFilename + " & " + fragFilename);
	}
	CHECK_GL_ERROR();

	bindAttributeLocation(Position::Location, "position");

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
	perVertexJointCountUniform = uniformLocation("perVertexJointCount");
	jointMatricesUniform = uniformLocation("jointMatrices");

	Debug{} << "\nSHADER " << vertFilename << " & " << fragFilename << "\nAttribute locations:\n\tposition=" << Position::Location;
}

}

