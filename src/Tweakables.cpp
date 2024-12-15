#include "Tweakables.h"

#include <sstream>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Utility/DebugStl.h>

#include "MagnumGameApp.h"
#include "Player.h"
#include "../../../deps/include/Corrade/Containers/EnumSet.h"

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
        Debug debug{&os};
        debug << modeName;
        for (size_t tweakableIndex = 0; tweakableIndex < tweakableValues.size(); tweakableIndex++) {
            auto &tweakableValue = tweakableValues[tweakableIndex];
            debug << Debug::newline << tweakableValue.name << tweakableValue.get();
            debug << (tweakableIndex == getCurrentTweakableIndex() ? "<-CURRENT" : "");
        }
    }


    void Tweakables::addDebugMode(const char *modeName, size_t initialIndex,
                                  std::initializer_list<TweakableValue> tweakableValues) {
        arrayAppend(_debugModes, InPlaceInit, modeName, initialIndex, tweakableValues);
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
        auto numDebugModes = _debugModes.size();
        _currentDebugModeIndex = (_currentDebugModeIndex + delta + numDebugModes) % numDebugModes;
    }

    static int getModifiersPower10Adjustment(MagnumGameApp::Modifiers modifiers) {
        auto powerAdjust = -1;
        if (modifiers & MagnumGameApp::Modifier::Shift) {
            powerAdjust -= 1;
        } else if (modifiers & (MagnumGameApp::Modifier::Ctrl | MagnumGameApp::Modifier::Alt)) {
            powerAdjust += 1;
        }
        return powerAdjust;
    }

    bool TweakableUIText::handleKeyPress(MagnumGameApp::Key key,
                                         MagnumGameApp::Modifiers modifiers) {
        using Key = MagnumGameApp::Key;

        switch (key) {
            case Key::PageDown:
                _tweakables.changeDebugModeIndex(1);
                return true;
            case Key::PageUp:
                _tweakables.changeDebugModeIndex(-1);
                return true;
            default: break;
        }

        auto &debugMode = _tweakables.currentDebugMode();
        if (debugMode.getModeName() && debugMode.hasTweakableValues()) {
            switch (key) {
                case Key::Left:
                    debugMode.currentTweaker().tweakBy(-1, getModifiersPower10Adjustment(modifiers));
                    return true;
                case Key::Right:
                    debugMode.currentTweaker().tweakBy(1, getModifiersPower10Adjustment(modifiers));
                    return true;
                case Key::Up:
                    debugMode.changeCurrentTweaker(-1);
                    return true;
                case Key::Down:
                    debugMode.changeCurrentTweaker(1);
                    return true;
            default: break;
            }
        }

        return false;
    }
}
