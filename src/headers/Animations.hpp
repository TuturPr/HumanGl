#pragma once
#include "Animation.hpp"
#include <cmath>
#include <glm/glm.hpp>

namespace Animations {

    inline AnimationFunc idle() {
        return [](float t) -> Animation {
            Animation a;
            a.headAngle = std::sin(t * 0.8f) * 3.0f;  // légère respiration
            return a;
        };
    }

    inline AnimationFunc walk() {
        return [](float t) -> Animation {
            Animation a;
            float speed = 3.0f, amp = 35.0f;
            a.armAngle_L    =  std::sin(t * speed) * amp;
            a.armAngle_R    = -std::sin(t * speed) * amp;
            // a.forearmAngle_L = std::max(0.0f,  std::sin(t * speed) * 20.0f);
            // a.forearmAngle_R = std::max(0.0f, -std::sin(t * speed) * 20.0f);
            a.legAngle_L    = -std::sin(t * speed) * amp;
            a.legAngle_R    =  std::sin(t * speed) * amp;
            // a.shinAngle_L   =  std::max(0.0f, -std::sin(t * speed) * 30.0f);
            // a.shinAngle_R   =  std::max(0.0f,  std::sin(t * speed) * 30.0f);
            a.headAngle     =  std::sin(t * speed * 0.5f) * 4.0f;
            return a;
        };
    }

    inline AnimationFunc jump() {
        return [](float t) -> Animation {
            Animation a;
            float phase = std::fmod(t, 1.2f) / 1.2f;  // cycle de 1.2s
            // Arc vertical
            a.bodyOffsetY = std::max(0.0f, std::sin(phase * glm::pi<float>()) * 1.5f);
            // Bras levés pendant le saut
            a.armAngle_L  = -60.0f + std::sin(phase * glm::pi<float>()) * -30.0f;
            a.armAngle_R  = -60.0f + std::sin(phase * glm::pi<float>()) * -30.0f;
            // Jambes pliées en l'air
            a.legAngle_L  = std::sin(phase * glm::pi<float>()) * -20.0f;
            a.legAngle_R  = std::sin(phase * glm::pi<float>()) * -20.0f;
            a.shinAngle_L = std::sin(phase * glm::pi<float>()) *  40.0f;
            a.shinAngle_R = std::sin(phase * glm::pi<float>()) *  40.0f;
            return a;
        };
    }

    // BONUS — facile d'en ajouter une nouvelle ici
    inline AnimationFunc disco() {
        return [](float t) -> Animation {
            Animation a;
            a.torsoAngle    = std::sin(t * 4.0f) * 20.0f;
            a.armAngle_L    = std::sin(t * 4.0f) * 80.0f;
            a.armAngle_R    = std::cos(t * 4.0f) * 80.0f;
            a.forearmAngle_L = std::abs(std::sin(t * 4.0f)) * 90.0f;
            a.forearmAngle_R = std::abs(std::cos(t * 4.0f)) * 90.0f;
            a.headAngle     = std::sin(t * 8.0f) * 15.0f;
            return a;
        };
    }
}
