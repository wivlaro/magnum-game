#pragma once

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/StaticArray.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Resource.h>

#include "MagnumGameCommon.h"

namespace MagnumGame {

class ShadowLight : public Object3D
{
public:
	ShadowLight(Object3D& parent, int numShadowLevels, Float near, Float far);
	~ShadowLight() override;

	void setTarget(Vector3 lightDirection, Vector3 screenDirection, const Matrix4& inverseModelViewProjection);
	
	void render(SceneGraph::DrawableGroup3D& drawables);

	size_t getNumLayers() const { return _layers.size(); }

	const Matrix4& getLayerMatrix(size_t layer) const { return _layers[layer].shadowMatrix; }

	const auto& getCutPlanes() const { return _cutPlanes; }

	GL::Texture2DArray& getShadowmapTextureArray() { return *_shadowTexture; }

private:
	Containers::Pointer<GL::Texture2DArray> _shadowTexture{};
	size_t _numLayers;
	Containers::Array<float> _cutPlanes{};
	SceneGraph::Camera3D& _camera;

	Containers::StaticArray<6, Vector4> _clipPlanes{};

	struct ShadowLayerData {
		GL::Framebuffer shadowFramebuffer;
		Matrix4 shadowCameraMatrix;
		Matrix4 shadowMatrix;
		Vector2 orthographicSize;
		float orthographicNear, orthographicFar;

		ShadowLayerData(Range2Di viewport) : shadowFramebuffer(viewport) { }
	};

	Containers::Array<ShadowLayerData> _layers{};

	void updateClipPlanes();

	Containers::StaticArray<8, Vector3> computeCameraFrustumCorners(int layer, Math::Matrix4<float> imvp);
};

}
