//
// Created by Bill Robinson on 17/07/2024.
//

#pragma once
#include <LinearMath/btVector3.h>

namespace MagnumGame {

    /**
     * @brief Interface for objects we want to be able to make visible/invisible
     */
    class IEnableDrawable {
    public:
        [[nodiscard]]
        virtual bool isEnabled() const = 0;
        virtual void setEnabled(bool shouldEnable) = 0;

        /**
         * @brief Decay away the enabled state of this object, for explosions, thrusters, etc.
         */
        virtual void decayEnabled() = 0;

        [[nodiscard]]
        virtual btVector3 getPosition() const = 0;
    protected:
        ~IEnableDrawable() = default;

    };
}
