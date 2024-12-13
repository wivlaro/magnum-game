#include "Tweakables.h"

#include <sstream>

#include "MagnumGameApp.h"
#include "Player.h"

namespace MagnumGame {


    Tweakables::Tweakables() {

        _debugModes.emplace_back();

        _debugModes.emplace_back(DebugMode{"Lights", 0, std::vector{
            TweakableValue{"Ambient", &TexturedDrawable::ambientColour},
            TweakableValue{"Directional", &TexturedDrawable::lightColour},
            TweakableValue{"Shininess", &TexturedDrawable::shininess},
            TweakableValue{"Specular", &TexturedDrawable::specular},
            TweakableValue{"Direction x", &TexturedDrawable::lightDirection.x()},
            TweakableValue{"Direction y", &TexturedDrawable::lightDirection.y()},
            TweakableValue{"Direction z", &TexturedDrawable::lightDirection.z()},
        }});
    }


    void Tweakables::addDebugMode(const char *modeName, size_t initialIndex, std::vector<TweakableValue>&& tweakableValues) {
        _debugModes.emplace_back(DebugMode{modeName, initialIndex, std::move(tweakableValues)});
    }


    std::string Tweakables::getDebugText() const {
        std::string debugText;
        auto& debugMode = currentDebugMode();
        if (debugMode.modeName) {
            std::ostringstream os;
            {
                Debug debug{&os};
                debug << debugMode.modeName;
                for (size_t tweakableIndex = 0; tweakableIndex < debugMode.tweakableValues.size(); tweakableIndex++)
                {
                    auto& tweakableValue = debugMode.tweakableValues[tweakableIndex];
                    debug << Debug::newline << tweakableValue.name << tweakableValue.get();
                    debug << (tweakableIndex == debugMode.currentTweakIndex ? "<-CURRENT" : "");
                }
            }
            debugText = os.str();
        }
        else {
            debugText = "";
        }
        return debugText;
    }

    void Tweakables::changeDebugModeIndex(int delta) {
        _currentDebugModeIndex = (_currentDebugModeIndex + delta + _debugModes.size()) % _debugModes.size();
    }

}
