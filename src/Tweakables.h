#pragma once
#include <functional>
#include <utility>
#include <vector>
#include <Corrade/Containers/Pointer.h>

namespace MagnumGame {
    class Tweakables {
    public:
        Tweakables();

        /**
         * @brief Named pointer for tweaking a configurable float value at runtime
         */
        class TweakableValue {
        public:
            const char *name;

            TweakableValue(const char *name, float *value)
                : TweakableValue{
                    name,
                    [value]() { return *value; },
                    [value](float v) { *value = v; }
                } {
            }

            TweakableValue(const char *name,
                           std::function<float()> getter,
                           std::function<void(float)> setter)
                : name(name), getter(std::move(getter)), setter(std::move(setter)) {
            }

        private:
            std::function<void(float)> setter;
            std::function<float()> getter;
        };

        /**
         * @brief Group of tweakable values for tuning
         */
        struct DebugMode {
            const char *modeName;
            size_t currentTweakIndex = 0;
            std::vector<TweakableValue> tweakableValues;
        };

        const DebugMode &currentDebugMode() const { return _debugModes[_currentDebugModeIndex]; }
        DebugMode &currentDebugMode() { return _debugModes[_currentDebugModeIndex]; }
        bool hasActiveDebugMode() const { return _currentDebugModeIndex > 0; }

        void addDebugMode(const char *modeName, size_t initialIndex, std::vector<TweakableValue> &&tweakableValues);

        std::string getDebugText() const;

        void changeDebugModeIndex(int delta);

    private:
        std::vector<DebugMode> _debugModes;
        int _currentDebugModeIndex = 0;
    };
}
