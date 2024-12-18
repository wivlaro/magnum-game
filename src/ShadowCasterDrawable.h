#pragma once
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Resource.h>
#include <Magnum/GL/GL.h>
#include <Magnum/Math/Range.h>

#include "Animator.h"
#include "MagnumGameCommon.h"

namespace MagnumGame {

class ShadowCasterShader;

class ShadowCasterDrawable : public SceneGraph::Drawable3D
{
public:
	explicit ShadowCasterDrawable(Object3D& parent, ShadowCasterShader &shader, SceneGraph::DrawableGroup3D &drawables);

	auto& setMesh(GL::Mesh* mesh) { this->mesh = mesh; return *this; }
	auto& setSkinMeshDrawable(SkinMeshDrawable skinMeshDrawable) { _skinMeshDrawable = skinMeshDrawable; return *this; }
	auto& setAABB(const Range3D& aabb) { this->_aabb = aabb; _aabbRadius = aabb.size().length() * 0.5f; return *this; }
	const Range3D& getAABB() const { return _aabb; }
	Float getAABBRadius() const { return _aabbRadius; }




	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

private:
	GL::Mesh* mesh;
	ShadowCasterShader& _shader;
	Range3D _aabb;
	Float _aabbRadius;

	SkinMeshDrawable _skinMeshDrawable;
};

}
