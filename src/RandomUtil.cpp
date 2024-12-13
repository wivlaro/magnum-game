//
// Created by Bill Robinson on 24/10/2024.
//

#include "RandomUtil.h"
#include <random>

namespace MagnumGame {

    static std::default_random_engine generator(std::random_device{}());
    static std::uniform_real_distribution<float> distribution01(0.0f, 1.0f);

    [[maybe_unused]]
    float random01() {
        return distribution01(generator);
    }

    [[maybe_unused]]
    float randomRange(float a, float b) {
        std::uniform_real_distribution<float> distribution(a,b);
        return distribution(generator);
    }

    int randomIndex(int size) {
        std::uniform_int_distribution<int> distribution(0, size-1);
        return distribution(generator);
    }

    float randomNegPos() {
        static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
        return distribution(generator);
    }

    bool randomChance(float chance) {
        if (chance <= 0) return false;
        if (chance >= 1) return true;
        return distribution01(generator) < chance;
    }

}