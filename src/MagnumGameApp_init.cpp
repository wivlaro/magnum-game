#include <cassert>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Path.h>
#include <Corrade/Containers/Pair.h>
#include <Corrade/Utility/String.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "Animator.h"
#include "GameState.h"
#include "MagnumGameApp.h"


namespace MagnumGame {


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
