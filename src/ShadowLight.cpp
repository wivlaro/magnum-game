#include "ShadowLight.h"
#include <Magnum/SceneGraph/FeatureGroup.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/TextureArray.h>
#include <Magnum/Math/Matrix3.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/SceneGraph/AbstractObject.h>
#include <Corrade/Containers/Reference.h>
#include <algorithm>
#include <unordered_set>
#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Scene.h>

#include "ShadowCasterDrawable.h"

namespace MagnumGame {

	using namespace Magnum::GL;

ShadowLight::ShadowLight(Object3D& parent, Range1D zPlanes, int numShadowLevels, Vector2i shadowMapSize)
:	Object3D(&parent)
	, _numLayers(numShadowLevels)
,	_camera(addFeature<SceneGraph::Camera3D>())
{
	Range2Di viewport = {{0, 0}, shadowMapSize};
	_shadowTexture.emplace();
#ifndef MAGNUM_TARGET_WEBGL
	_shadowTexture->setLabel("Shadow texture");
#endif
	//	shadowTexture.setStorage(1, Magnum::TextureFormat::DepthComponent, shadowFramebuffer.viewport().size());
	_shadowTexture->setImage(0, TextureFormat::DepthComponent, ImageView3D(GL::PixelFormat::DepthComponent, PixelType::Float, {viewport.size(), static_cast<int>(_numLayers)}, nullptr));
	_shadowTexture->setMaxLevel(0);

	_shadowTexture->setCompareFunction(SamplerCompareFunction::LessOrEqual);
	_shadowTexture->setCompareMode(SamplerCompareMode::CompareRefToTexture);

	_shadowTexture->setMinificationFilter(GL::SamplerFilter::Linear, GL::SamplerMipmap::Base);
	_shadowTexture->setMagnificationFilter(GL::SamplerFilter::Linear);

	arrayReserve(_layers, _numLayers);
	for (auto i = 0u; i < _numLayers; i++) {
		auto& layer = arrayAppend(_layers, InPlaceInit, viewport);
		auto& shadowFramebuffer = layer.shadowFramebuffer;
#ifndef MAGNUM_TARGET_WEBGL
		shadowFramebuffer.setLabel("Shadow framebuffer " + std::to_string(i));
#endif
		shadowFramebuffer.bind();
		shadowFramebuffer.attachTextureLayer(Framebuffer::BufferAttachment::Depth, *_shadowTexture, 0, i);
		shadowFramebuffer.mapForDraw(Framebuffer::DrawAttachment::None);
		Debug() << "Framebuffer status: read=" << shadowFramebuffer.checkStatus(FramebufferTarget::Read) << " draw=" << shadowFramebuffer.checkStatus(FramebufferTarget::Draw);
	}

	auto zNear = zPlanes.min();
	auto zFar = zPlanes.max();
	_cutPlanes = {};
	arrayReserve(_cutPlanes, _numLayers);
	//props http://stackoverflow.com/a/33465663
	for (auto i = 1u; i <= _numLayers; i++) {
		//		float linearDepth = zNear + i * (zFar - zNear) / numLayers;
		//		float linearDepth = zNear + (numLayers - i) * (zFar) / numLayers;
		float linearDepth = zNear + std::pow(static_cast<float>(i) / _numLayers, 3.0f) * (zFar - zNear);
		float nonLinearDepth = (zFar + zNear - 2.0f * zNear * zFar / linearDepth) / (zFar - zNear);
		arrayAppend(_cutPlanes, (nonLinearDepth + 1.0f) / 2.0f);
	}
}

ShadowLight::~ShadowLight() = default;

void ShadowLight::setTarget(Vector3 lightDirection, Vector3 screenDirection, const Matrix4& inverseModelViewProjection)
{
	auto cameraMatrix = Matrix4::lookAt({0,0,0}, -lightDirection, screenDirection);
	auto cameraRotationMatrix = cameraMatrix.rotation();
	auto inverseCameraRotationMatrix = cameraRotationMatrix.inverted();


	for (auto i = 0u; i < _numLayers; i++) {
		auto mainCameraFrustumCorners = computeCameraFrustumCorners(i, inverseModelViewProjection);
		auto& d = _layers[i];
		Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
		for (auto worldPoint : mainCameraFrustumCorners) {
			auto cameraPoint = inverseCameraRotationMatrix * worldPoint;
			for (size_t i = 0; i < 3; i++) {
				if (cameraPoint[i] < min[i]) {
					min[i] = cameraPoint[i];
				}
				if (cameraPoint[i] > max[i]) {
					max[i] = cameraPoint[i];
				}
			}
		}
		auto mid = (min+max) * 0.5f;

		auto cameraPosition = cameraRotationMatrix * mid;

		auto range = max - min;
		d.orthographicSize = range.xy();
		d.orthographicNear = -0.5f * range.z();
		d.orthographicFar =  0.5f * range.z();
		cameraMatrix.translation() = cameraPosition;
		d.shadowCameraMatrix = cameraMatrix;
	}
}

Containers::StaticArray<8, Vector3> ShadowLight::computeCameraFrustumCorners(int layer, Math::Matrix4<float> imvp) {
	auto projectImvpAndDivide = [&](Vector4 vec) -> Vector3 {
		auto vec2 = imvp * vec;
		return vec2.xyz() / vec2.w();
	};
	auto z0 = layer == 0 ? 0 : _cutPlanes[layer-1];
	auto z1 = _cutPlanes[layer];
	return {
		projectImvpAndDivide({-1,-1, z0, 1}),
		projectImvpAndDivide({ 1,-1, z0, 1}),
		projectImvpAndDivide({-1, 1, z0, 1}),
		projectImvpAndDivide({ 1, 1, z0, 1}),
		projectImvpAndDivide({-1,-1, z1, 1}),
		projectImvpAndDivide({ 1,-1, z1, 1}),
		projectImvpAndDivide({-1, 1, z1, 1}),
		projectImvpAndDivide({ 1, 1, z1, 1}),
	};
}

void ShadowLight::render(SceneGraph::DrawableGroup3D& drawables)
{
	/* Compute transformations of all objects in the group relative to the camera */
	static std::vector<std::reference_wrapper<SceneGraph::AbstractObject3D>> objects{};
	objects.reserve(drawables.size());
	for (size_t i = 0; i < drawables.size(); i++) {
		objects.emplace_back(drawables[i].object());
	}
	static Containers::Array<ShadowCasterDrawable*> filteredDrawables;
	arrayReserve(filteredDrawables, drawables.size());

	auto bias = Matrix4{
			{0.5f, 0.0f, 0.0f, 0.0f},
			{0.0f, 0.5f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.5f, 0.0f},
			{0.5f, 0.5f, 0.5f, 1.0f}
	};

	for (auto layer = 0u; layer < _numLayers; layer++) {
		auto& d = _layers[layer];
		auto orthographicNear = d.orthographicNear;
		auto orthographicFar = d.orthographicFar;
		setTransformation(d.shadowCameraMatrix);
		setClean();
		_camera.setProjectionMatrix(Matrix4::orthographicProjection(d.orthographicSize, orthographicNear, orthographicFar));
		updateClipPlanes();

		auto transformations = scene()->AbstractObject<3,Type>::transformationMatrices(objects, _camera.cameraMatrix());
		auto transformationsOutIndex = 0u;

		filteredDrawables = {};
		static std::unordered_set<int> clippedDrawables;
		for (size_t drawableIndex = 0; drawableIndex < drawables.size(); drawableIndex++) {
			auto& drawable = static_cast<ShadowCasterDrawable&>(drawables[drawableIndex]);
			auto& aabb = drawable.getAABB();
			auto radius = drawable.getAABBRadius();
			Vector4 localCentre(aabb.center(), 1);
			auto transform = transformations[drawableIndex];
			Vector4 drawableCentre = transform * localCentre;
			for (size_t clipPlaneIndex = 1; clipPlaneIndex < _clipPlanes.size(); clipPlaneIndex++) {
				auto distance = Math::dot(_clipPlanes[clipPlaneIndex], drawableCentre);
				if (distance < -radius) {
					clippedDrawables.erase(drawableIndex);
					goto next;
				}
			}
			clippedDrawables.insert(drawableIndex);
			{
				/* If this object extends in front of the near plane, extend the near plane.
				 * We negate the z because the negative z is forward away from the camera,
				 * but the near/far planes are measured forwards. */
				auto nearestPoint = - drawableCentre.z() - radius;
				if (nearestPoint < orthographicNear) {
					orthographicNear = nearestPoint;
				}
				arrayAppend(filteredDrawables, &drawable);
				transformations[transformationsOutIndex++] = transform;
			}
			next:;
		}

		auto shadowCameraProjectionMatrix = Matrix4::orthographicProjection(d.orthographicSize, orthographicNear, orthographicFar);
		d.shadowMatrix = bias * shadowCameraProjectionMatrix * _camera.cameraMatrix();
		_camera.setProjectionMatrix(shadowCameraProjectionMatrix);

		d.shadowFramebuffer.clear(FramebufferClear::Depth);
		d.shadowFramebuffer.bind();
		for(auto i = 0U; i != transformationsOutIndex; ++i) {
			filteredDrawables[i]->draw(transformations[i], _camera);
		}
	}

	defaultFramebuffer.bind();

	objects.clear();
	filteredDrawables = {};
}

void ShadowLight::updateClipPlanes()
{
	auto pm = _camera.projectionMatrix();
	_clipPlanes = {
		Vector4(pm[3][0] + pm[2][0], pm[3][1] + pm[2][1], pm[3][2] + pm[2][2], pm[3][3] + pm[2][3] ), // near
		Vector4(pm[3][0] - pm[2][0], pm[3][1] - pm[2][1], pm[3][2] - pm[2][2], pm[3][3] - pm[2][3] ), // far

		Vector4(pm[3][0] + pm[0][0], pm[3][1] + pm[0][1], pm[3][2] + pm[0][2], pm[3][3] + pm[0][3] ), // left
		Vector4(pm[3][0] - pm[0][0], pm[3][1] - pm[0][1], pm[3][2] - pm[0][2], pm[3][3] - pm[0][3] ), // right

		Vector4(pm[3][0] + pm[1][0], pm[3][1] + pm[1][1], pm[3][2] + pm[1][2], pm[3][3] + pm[1][3] ), // bottom
		Vector4(pm[3][0] - pm[1][0], pm[3][1] - pm[1][1], pm[3][2] - pm[1][2], pm[3][3] - pm[1][3] ), // top
	};
	for (auto& plane : _clipPlanes) {
		plane *= plane.xyz().lengthInverted();
	}
}

}
