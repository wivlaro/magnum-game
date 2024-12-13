#include "GameState.h"

#include <sstream>
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

#include "GameAssets.h"
#include "OnGroundQuery.h"
#include "Player.h"

namespace MagnumGame {
    GameState::GameState(Timeline& timeline, GameAssets& assets) : _timeline(timeline), _assets(assets) {
        _bSphereQueryObject.setCollisionShape(&_bSphereQueryShape);

        _bWorld.setGravity({0.0f, -10.0f, 0.0f});

        _debugDraw = BulletIntegration::DebugDraw{};
        _debugDraw.setMode(BulletIntegration::DebugDraw::Mode::DrawWireframe);
        _bWorld.setDebugDrawer(&_debugDraw);


        auto& cameraObject = _scene.addChild<Object3D>(nullptr)
            .translate(Vector3::zAxis(30.0f))
            .rotateX(-90.0_degf);
        auto& camera = cameraObject.addFeature<SceneGraph::Camera3D>();
        camera.setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
            .setProjectionMatrix(Matrix4::perspectiveProjection(30.0_degf, 1.0f, 0.1f, 100.0f))
            .setViewport(GL::defaultFramebuffer.viewport().size());

        _cameraController.emplace(cameraObject, camera);
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
        _levelShapes.clear();
        _levelMeshes.clear();
        _meshToShapeMap.clear();

        std::vector<UnsignedInt> meshToMeshCollider{};
        for (auto meshId = 0U; meshId < importer.meshCount(); meshId++) {
            meshToMeshCollider.push_back(meshId);
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
                _levelShapes.emplace_back(InPlaceInit, meshPositions.data()->data(),
                                          static_cast<int>(meshPositions.size()), sizeof(Vector3));
                _levelMeshes.emplace_back(InPlaceInit, NoCreate);
            } else {
                auto colliderName = meshName + colliderSuffix;
                auto colliderId = importer.meshForName(colliderName);
                bool hasCollider = colliderId != -1;
                if (hasCollider) {
                    meshToMeshCollider[meshId] = colliderId;
                    _levelShapes.emplace_back(InPlaceInit);
                    debug << "has a collider" << colliderId << colliderName;
                } else {
                    auto meshPositions = meshData->positions3DAsArray();
                    _levelShapes.emplace_back(InPlaceInit, meshPositions.data()->data(),
                                              static_cast<int>(meshPositions.size()), sizeof(Vector3));
                    debug << "has no explicit collider, creating from mesh";
                }
                _levelMeshes.emplace_back(InPlaceInit, MeshTools::compile(*meshData))->setLabel(meshName);
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
        rigidBody->rigidBody().setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));

        _player = std::make_unique<Player>("Someone", rigidBody, animator);
        _player->resetToStart(Matrix4::translation({0,2,0}));
        _cameraController->setupTargetFromCurrent(*rigidBody);

        animator->play("idle", false);
    }

    void GameState::renderDebug(const Matrix4 &transformationProjectionMatrix) {
        _debugDraw.setTransformationProjectionMatrix(transformationProjectionMatrix);
        _bWorld.debugDrawWorld();
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

        _cameraController->draw(_animatorDrawables);

    }
}
