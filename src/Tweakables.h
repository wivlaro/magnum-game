#pragma once
#include <functional>
#include <Corrade/Containers/Array.h>

#include "MagnumGameApp.h"
#include "UserInterface.h"

namespace MagnumGame {
    class Tweakables {
    public:
        explicit Tweakables();

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

            float get() const { return getter(); }
            void set(float v) { setter(v); }

            void tweakBy(int sign, int powerAdjust) {
                float value = get();

                float absValue = std::max(abs(value), 0.00001f);
                //Compute the nearest sensible power of 10 to for adjusting a value
                set(value + sign * pow(10, round(log10(absValue)) + powerAdjust));
            }

        private:
            std::function<float()> getter;
            std::function<void(float)> setter;
        };

        /**
         * @brief Group of tweakable values for tuning
         */
        class DebugMode {
        public:
            explicit DebugMode() : modeName{nullptr} { }
            explicit DebugMode(const char *mode_name, size_t current_tweak_index,
                std::initializer_list<TweakableValue> tweakable_values);

            const char* getModeName() const { return modeName; }

            TweakableValue &currentTweaker() {
                return tweakableValues[getCurrentTweakableIndex()];
            }

            // const Corrade::Containers::Array<TweakableValue> &getTweakableValues() const { return tweakableValues; }

            void changeCurrentTweaker(int delta) {
                auto size = tweakableValues.size();
                currentTweakIndex = (currentTweakIndex + delta + size) % size;
            }

            size_t getCurrentTweakableIndex() const { return currentTweakIndex % tweakableValues.size(); }

            void printTweakables(std::ostringstream & os) const;

            bool hasTweakableValues() const { return tweakableValues.size() > 0; }

        private:
            const char *modeName;
            size_t currentTweakIndex = 0;
            Containers::Array<TweakableValue> tweakableValues;

        };

        const DebugMode &currentDebugMode() const { return _debugModes[_currentDebugModeIndex]; }
        DebugMode &currentDebugMode() { return _debugModes[_currentDebugModeIndex]; }
        bool hasActiveDebugMode() const { return _currentDebugModeIndex > 0; }

        void addDebugMode(const char *modeName, size_t initialIndex, std::initializer_list<TweakableValue> tweakableValues);

        void printTweakables(const DebugMode &debugMode, std::ostringstream os) const;

        std::string getDebugText() const;

        void changeDebugModeIndex(int delta);

    private:
        Containers::Array<DebugMode> _debugModes;
        int _currentDebugModeIndex = 0;
    };

    class TweakableUIText : public UIText {
    public:
        explicit TweakableUIText(TextAsset &font, const Vector2 &position, float size, Text::Alignment alignment, Tweakables& tweakables)
        : UIText(font,position,size,alignment)
        , _tweakables(tweakables) {}
        bool handleKeyPress(MagnumGameApp::Key, MagnumGameApp::Modifiers modifiers) override;
    private:
        Tweakables& _tweakables;
    };
}
