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
#include <Magnum/Trade/LightData.h>
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
#include "GameAssets.h"
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
        _assets.emplace(*gltfImporter);

        _gameState.emplace(_timeline, *_assets);
        _gameState->loadLevel(*gltfImporter);
        _gameState->setupPlayer();
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
};
