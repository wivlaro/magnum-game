#pragma once
#include <vector>

namespace MagnumGame {

class Tweakables {
public:
    Tweakables();

    /**
     * @brief Named pointer for tweaking a configurable float value at runtime
     */
    struct TweakableValue {
        const char* name;
        float* pValue;
    };

    /**
     * @brief Group of tweakable values for tuning
     */
    struct DebugMode {
        const char* modeName;
        size_t currentTweakIndex = 0;
        std::vector<TweakableValue> tweakableValues;
    };
    const DebugMode& currentDebugMode() const { return _debugModes[_currentDebugModeIndex]; }
    DebugMode& currentDebugMode() { return _debugModes[_currentDebugModeIndex]; }
    bool hasActiveDebugMode() const {return _currentDebugModeIndex > 0;}

    void addDebugMode(const char* modeName, size_t initialIndex, std::vector<TweakableValue>&& tweakableValues);
    std::string getDebugText() const;

    void changeDebugModeIndex(int delta);

private:
    std::vector<DebugMode> _debugModes;
    int _currentDebugModeIndex = 0;

};

}
