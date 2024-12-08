#include <cassert>
#include <unordered_set>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Path.h>
#include <Corrade/Containers/Pair.h>
#include <Corrade/Utility/String.h>
#include "Corrade/Containers/StructuredBindings.h"
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MaterialData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>
#include <Magnum/Trade/CameraData.h>
#include <Magnum/Trade/AnimationData.h>
#include <Magnum/Trade/SkinData.h>
#include <Magnum/Math/CubicHermite.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/ImageView.h>
#include <Magnum/SceneGraph/Camera.h>

#include "Animator.h"
#include "GameState.h"
#include "GameModels.h"
#include "MagnumGameApp.h"
#include "Player.h"
#include "RigidBody.h"
#include "TexturedDrawable.h"
#include "UnlitAlphaDrawable.h"


namespace MagnumGame {
    void MagnumGameApp::setup() {
        PluginManager::Manager<Trade::AbstractImporter> manager;

        setupTextRenderer();


        auto gltfImporter = manager.loadAndInstantiate("GltfImporter");
        assert(gltfImporter);
        _models = std::make_unique<GameModels>(*gltfImporter);

        auto loadedModel = loadAnimatedModel(*gltfImporter, "characters/character-female-b.glb");
        _player = loadedModel.first();
        _playerAnimator = loadedModel.second();
        loadLevel(*gltfImporter);

        _gameState = std::make_unique<GameState>();

        _playerAnimator->play("idle");
    }


    Containers::Optional<Containers::String> MagnumGameApp::findDirectory(Containers::StringView dirName) {
        using namespace Corrade::Utility;

        if (auto currentDir = Path::currentDirectory()) {
            auto dir = *currentDir;
            while (true) {
                auto candidateDir = Path::join(dir, dirName);
                if (Path::exists(candidateDir) && Path::isDirectory(candidateDir)) {
                    Debug{} << "Found" << candidateDir << "directory";
                    return candidateDir;
                }
                auto parentDir = Path::split(dir).first();
                if (dir == parentDir) {
                    break;
                }
                dir = parentDir;
            }
        }
        Warning{} << "Found no" << dirName << "directory, looking in upwards from current" << Path::currentDirectory();
        return {};
    }


    Containers::Pair<Object3D*, Animator*> MagnumGameApp::loadAnimatedModel(Trade::AbstractImporter &importer, Containers::StringView fileName) {

        if (auto modelsDir = findDirectory("models")) {
            auto filePath = Utility::Path::join(*modelsDir, fileName);
            if (!importer.openFile(filePath)) {
                Error{} << "Can't open" << filePath << "with" << importer.plugin();
                return {};
            }
            if (importer.sceneCount() == 0) {
                Error{} << "No scene found in" << filePath;
                return {};
            }
            if (importer.sceneCount() > 1) {
                Warning{} << "Multiple scenes found in" << filePath << ", using the default one" << importer.defaultScene() << importer.sceneName(importer.defaultScene());
            }
        }

        auto& rootObject = _scene.addChild<Object3D>();
        auto& animator = rootObject.addFeature<Animator>(importer, _animatedTexturedShader, &_animatorDrawables, &_drawables);
        return {&rootObject, &animator};
    }

    void MagnumGameApp::loadLevel(Trade::AbstractImporter &importer) {
        const auto colliderSuffix = "-collider";

        if (auto modelsDir = findDirectory("models/levels")) {
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
        for (auto meshId = 0; meshId < importer.meshCount(); meshId++) {
            meshToMeshCollider.push_back(meshId);
        }

        Debug{} << "Meshes:" << importer.meshCount();
        for (auto meshId = 0; meshId < importer.meshCount(); meshId++) {
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

        for (auto meshId = 0; meshId < importer.meshCount(); meshId++) {
            auto shapeId = meshToMeshCollider[meshId];
            auto mesh = _levelMeshes[meshId].get();
            auto shape = _levelShapes[shapeId].get();
            _meshToShapeMap[mesh] = shape;
            Debug{} << "Mesh" << meshId << importer.meshName(meshId) << "has shape" << shapeId << importer.meshName(shapeId);
        }

        _levelTextures = GameModels::loadTextures(importer);

        auto materialTextures = GameModels::loadMaterials(importer, _levelTextures);

        for (UnsignedInt sc = 0; sc < importer.sceneCount(); sc++) {
            Debug{} << "Scene" << sc << ":" << importer.sceneName(sc) << "(default"
                    << importer.defaultScene() << ")";
            auto sceneData = importer.scene(sc);

            for (auto &objectId: sceneData->childrenFor(-1)) {
                auto objectName = importer.objectName(objectId);
                Debug{} << "\tObject" << objectId << objectName;

                for (auto cameraId: sceneData->camerasFor(objectId)) {
                    if (auto cameraData = importer.camera(cameraId)) {\
                        if (auto matrix = sceneData->transformation3DFor(objectId)) {
                            _cameraObject->setTransformation(*matrix);
                        }
                        auto projection = Matrix4::perspectiveProjection(
                            cameraData->fov(),
                            cameraData->aspectRatio(),
                            cameraData->near(),
                            cameraData->far());
                        _camera->setProjectionMatrix(projection);
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
                    auto &rigidBody = _scene.addChild<RigidBody>(0.0f, shape, _bWorld);

                    if (auto matrix = sceneData->transformation3DFor(objectId)) {
                        rigidBody.setTransformation(*matrix);
                        rigidBody.syncPose();
                    }

                    rigidBody.addFeature<TexturedDrawable>(materialTextures[materialId], _texturedShader, *mesh, _drawables);
                }
            }
        }
        { GL::Renderer::Error err; while ((err = GL::Renderer::error()) != GL::Renderer::Error::NoError) { Error() << __FILE__ << ":" << __LINE__ << "Error: " << err; } }
    }


    [[maybe_unused]]
    static void
    DumpNodeTree(Trade::AbstractImporter &importer, const Trade::SceneData &sceneData, Long parentId,
                 int level = 0) {
        for (auto childId: sceneData.childrenFor(parentId)) {
            auto log = Debug{};
            for (int i = 0; i < level; i++) {
                log << "  ";
            }
            log << importer.objectName(childId);
            log << sceneData.transformation3DFor(childId);
            Debug::newline(log);
            DumpNodeTree(importer, sceneData, childId, level + 1);
        }
    }
};
