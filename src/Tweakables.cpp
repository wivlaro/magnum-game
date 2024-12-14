#include "Tweakables.h"

#include <sstream>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Utility/DebugStl.h>

#include "MagnumGameApp.h"
#include "Player.h"

namespace MagnumGame {
    Tweakables::Tweakables()
        : _debugModes{} {
        arrayAppend(_debugModes, InPlaceInit);

        arrayAppend(_debugModes, InPlaceInit, "Lights", 0, std::initializer_list<TweakableValue>{
                        TweakableValue{"Ambient", &TexturedDrawable::ambientColour},
                        TweakableValue{"Directional", &TexturedDrawable::lightColour},
                        TweakableValue{"Shininess", &TexturedDrawable::shininess},
                        TweakableValue{"Specular", &TexturedDrawable::specular},
                        TweakableValue{"Direction x", &TexturedDrawable::lightDirection.x()},
                        TweakableValue{"Direction y", &TexturedDrawable::lightDirection.y()},
                        TweakableValue{"Direction z", &TexturedDrawable::lightDirection.z()},
                    });
    }

    Tweakables::DebugMode::DebugMode(const char *mode_name, size_t current_tweak_index,
                                     std::initializer_list<TweakableValue> tweakable_values)
        : modeName(mode_name),
          currentTweakIndex(current_tweak_index) {
        arrayAppend(tweakableValues, tweakable_values);
    }

    void Tweakables::DebugMode::printTweakables(std::ostringstream &os) const {
        Corrade::Utility::Debug debug{&os};
        debug << modeName;
        for (size_t tweakableIndex = 0; tweakableIndex < tweakableValues.size(); tweakableIndex++) {
            auto &tweakableValue = tweakableValues[tweakableIndex];
            debug << Corrade::Utility::Debug::newline << tweakableValue.name << tweakableValue.get();
            debug << (tweakableIndex == getCurrentTweakableIndex() ? "<-CURRENT" : "");
        }
    }


    void Tweakables::addDebugMode(const char *modeName, size_t initialIndex,
                                  std::initializer_list<TweakableValue> tweakableValues) {
        arrayAppend(_debugModes, InPlaceInit, modeName, initialIndex, std::move(tweakableValues));
    }

    std::string Tweakables::getDebugText() const {
        std::string debugText;
        auto &debugMode = currentDebugMode();
        if (debugMode.getModeName()) {
            std::ostringstream os;
            debugMode.printTweakables(os);
            debugText = os.str();
        } else {
            debugText = "";
        }
        return debugText;
    }

    void Tweakables::changeDebugModeIndex(int delta) {
        _currentDebugModeIndex = (_currentDebugModeIndex + delta + _debugModes.size()) % _debugModes.size();
    }
}
