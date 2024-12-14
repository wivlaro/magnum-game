#include "GameState.h"

#include <sstream>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Utility/DebugStl.h>
#include <Corrade/Utility/Path.h>
#include <Corrade/Utility/String.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/LightData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/GL/Renderer.h>
#include <Corrade/Containers/StructuredBindings.h>
#include <Magnum/DebugTools/ObjectRenderer.h>

#include "GameAssets.h"
#include "OnGroundQuery.h"
#include "Player.h"

namespace MagnumGame {
    GameState::GameState(const Timeline& timeline, GameAssets& assets)
    : _timeline(timeline)
    , _assets(assets) {
        _bSphereQueryObject.setCollisionShape(&_bSphereQueryShape);

        _bWorld.setGravity({0.0f, -10.0f, 0.0f});

        _debugDraw = BulletIntegration::DebugDraw{};
        _debugDraw.setMode(BulletIntegration::DebugDraw::Mode::DrawWireframe);
        _bWorld.setDebugDrawer(&_debugDraw);

        _cameraController.emplace(_scene);

        _debugResourceManager.set(DebugRendererGroup, DebugTools::ObjectRendererOptions{}.setSize(1.f));
    }

    void GameState::setControl(Vector2 controlVector) {

        if (_player) {
            _player->setControl(controlVector, _cameraController->getCameraObjectMatrix());
        }
    }

    void GameState::loadLevel(Trade::AbstractImporter &importer) {

        const auto colliderSuffix = "-collider";

        if (auto modelsDir = MagnumGameApp::findDirectory("models/levels")) {
            auto filePath = Utility::Path::join(*modelsDir, "level-1.glb");
            if (!importer.openFile(filePath)) {
                Warning{} << "Can't open" << filePath << "with" << importer.plugin();
                return;
            }
        }

        arrayRemove(_levelShapes, 0, _levelShapes.size());
        arrayRemove(_levelMeshes, 0, _levelMeshes.size());
        arrayReserve(_levelShapes, importer.meshCount());
        arrayReserve(_levelMeshes, importer.meshCount());

        _meshToShapeMap.clear();

        Containers::Array<UnsignedInt> meshToMeshCollider{NoInit, importer.meshCount()};
        for (auto meshId = 0U; meshId < importer.meshCount(); meshId++) {
            meshToMeshCollider[meshId] = meshId;
        }

        Debug{} << "Meshes:" << importer.meshCount();
        for (auto meshId = 0U; meshId < importer.meshCount(); meshId++) {
            auto meshName = importer.meshName(meshId);
            Debug debug{};
            debug << "\tMesh" << meshId << ":" << meshName;
            auto meshData = importer.mesh(meshId);

            bool isCollider = Utility::String::endsWith(colliderSuffix, meshName);
            if (isCollider) {
                auto normalMeshName = Utility::String::stripSuffix(meshName, colliderSuffix);
                auto normalMeshId = importer.meshForName(normalMeshName);
                if (normalMeshId != -1) {
                    debug << "is a collider for" << normalMeshId << normalMeshName;
                    meshToMeshCollider[normalMeshId] = meshId;
                } else {
                    debug << "is a standalone collider";
                }
                auto meshPositions = meshData->positions3DAsArray();
                meshToMeshCollider[meshId] = normalMeshId;
                arrayAppend(_levelShapes, InPlaceInit, InPlaceInit, meshPositions.data()->data(), static_cast<int>(meshPositions.size()), sizeof(Vector3));
                arrayAppend(_levelMeshes, InPlaceInit, InPlaceInit, NoCreate);
            } else {
                auto colliderName = meshName + colliderSuffix;
                auto colliderId = importer.meshForName(colliderName);
                bool hasCollider = colliderId != -1;
                if (hasCollider) {
                    meshToMeshCollider[meshId] = colliderId;
                    arrayAppend(_levelShapes, InPlaceInit);
                    debug << "has a collider" << colliderId << colliderName;
                } else {
                    auto meshPositions = meshData->positions3DAsArray();
                    arrayAppend(_levelShapes, InPlaceInit, InPlaceInit, meshPositions.data()->data(),
                                              static_cast<int>(meshPositions.size()), sizeof(Vector3));
                    debug << "has no explicit collider, creating from mesh";
                }
                [[maybe_unused]]
                auto& mesh = arrayAppend(_levelMeshes, InPlaceInit, InPlaceInit, MeshTools::compile(*meshData));
#ifndef MAGNUM_TARGET_WEBGL
                mesh->setLabel(meshName);
#endif
            }
        }

        for (auto meshId = 0U; meshId < importer.meshCount(); meshId++) {
            auto shapeId = meshToMeshCollider[meshId];
            auto mesh = _levelMeshes[meshId].get();
            auto shape = _levelShapes[shapeId].get();
            _meshToShapeMap[mesh] = shape;
            Debug{} << "Mesh" << meshId << importer.meshName(meshId) << "has shape" << shapeId << importer.meshName(shapeId);
        }

        _levelTextures = GameAssets::loadTextures(importer);

        _levelMaterials = GameAssets::loadMaterials(importer, _levelTextures);

        for (UnsignedInt sc = 0; sc < importer.sceneCount(); sc++) {
            Debug{} << "Scene" << sc << ":" << importer.sceneName(sc) << "(default"
                    << importer.defaultScene() << ")";
            auto sceneData = importer.scene(sc);

            for (auto &objectId: sceneData->childrenFor(-1)) {
                auto objectName = importer.objectName(objectId);
                Debug{} << "\tObject" << objectId << objectName;

                for (auto cameraId: sceneData->camerasFor(objectId)) {
                    if (auto cameraData = importer.camera(cameraId)) {
                        _cameraController->loadCameraData(sceneData->transformation3DFor(objectId), *cameraData);
                    }
                }

                for (auto light : sceneData->lightsFor(objectId)) {
                    auto lightData = importer.light(light);

                    Debug debug{};
                    debug << "\tLight" << objectId << light
                    << lightData->type()
                                  << "atten" << lightData->attenuation()
                    << "color" << lightData->color()
                    << "intensity" << lightData->intensity()
                    << "range" <<lightData->range()
                    << "cone angles" << lightData->innerConeAngle() << lightData->outerConeAngle();

                    if (auto matrix = sceneData->transformation3DFor(objectId)) {
                        debug << "\tTransformation" << matrix->translation();
                    }
                }

                if (Utility::String::endsWith(objectName, colliderSuffix)) {
                    continue;
                }

                for (auto &[meshId,materialId]: sceneData->meshesMaterialsFor(objectId)) {
                    Debug{} << "\t\tMesh" << meshId << importer.meshName(meshId) << "Material" <<
                            materialId << (materialId == -1 ? "NONE" : importer.materialName(materialId));

                    auto mesh = _levelMeshes[meshId].get();
                    auto shape = _meshToShapeMap[mesh];
                    auto &rigidBody = _scene.addChild<RigidBody>(0.0f, shape, _bWorld, RigidBody::CollisionLayer::Terrain);


                    if (auto matrix = sceneData->transformation3DFor(objectId)) {
                        rigidBody.setTransformation(*matrix);
                        rigidBody.syncPose();
                    }

                    rigidBody.addFeature<TexturedDrawable>(_levelMaterials[materialId].texture, _assets.getTexturedShader(), *mesh, _opaqueDrawables);
                }
            }
        }

        CHECK_GL_ERROR(__FILE__,__LINE__);
    }

    void GameState::drawOpaque() {
        _cameraController->draw(_opaqueDrawables);
    }

    void GameState::drawTransparent() {
        //Might want to sort the drawables along the camera Z axis
        _cameraController->draw(_transparentDrawables);
    }

    void GameState::setupPlayer() {
        RigidBody *rigidBody = &_scene.addChild<RigidBody>(1.0f, &_assets.getPlayerShape(), _bWorld,
                                                           RigidBody::CollisionLayer::Dynamic);
        auto& animationOffset = rigidBody->addChild<Object3D>();
        Animator *animator = &animationOffset.addFeature<Animator>(*_assets.getPlayerAsset(), _assets.getAnimatedTexturedShader(),
                                                                   &_animatorDrawables, &_opaqueDrawables);
        animationOffset.setTransformation(Matrix4::translation({0, -0.4f, 0}));

        //Prevent rotation in X & Z
        rigidBody->rigidBody().setAngularFactor(btVector3(0.0f, 0.5f, 0.0f));
        Debug{} << "Friction: " << rigidBody->rigidBody().getFriction() << "rolling friction" << rigidBody->rigidBody().getRollingFriction();
        rigidBody->rigidBody().setRollingFriction(0.5f);

        _player.emplace("Someone", rigidBody, animator);
        _player->resetToStart(Matrix4::translation({0,2,0}));
        _cameraController->setupTargetFromCurrent(*rigidBody);

        animator->play("idle", false);
    }

    void GameState::renderDebug(const Matrix4 &transformationProjectionMatrix) {
        _debugDraw.setTransformationProjectionMatrix(transformationProjectionMatrix);
        _bWorld.debugDrawWorld();

        if (!_debugDrawables.isEmpty()) {
            _cameraController->draw(_debugDrawables);
        }
    }

    void GameState::start() {
        _isStarted = true;
    }

    void GameState::update() {
        if (_player) _player->update(_timeline.previousFrameDuration());

        _bWorld.stepSimulation(_timeline.previousFrameDuration(), 5);

        if (_cameraController) {
            _cameraController->update(_timeline.previousFrameDuration());
        }

        //Doesn't actually draw, but updates the bone matrices
        _cameraController->draw(_animatorDrawables);
    }


    void GameState::addDebugDrawable(SceneGraph::AbstractObject3D &playerRigidBody) {
        //This is a feature added to playerRigidBody - it is deleted with that. Not nice to allocate in app code and delete in library code.
        new DebugTools::ObjectRenderer3D{
            _debugResourceManager, playerRigidBody, DebugRendererGroup, &_debugDrawables
        };
    }

}
