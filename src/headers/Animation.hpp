#pragma once
#include <functional>

struct Animation {
    float torsoAngle    = 0.0f;
    float headAngle     = 0.0f;
    float armAngle_L    = 0.0f;
    float armAngle_R    = 0.0f;
    float forearmAngle_L = 0.0f;
    float forearmAngle_R = 0.0f;
    float legAngle_L    = 0.0f;
    float legAngle_R    = 0.0f;
    float shinAngle_L   = 0.0f;
    float shinAngle_R   = 0.0f;

    float bodyOffsetY   = 0.0f;
};

using AnimationFunc = std::function<Animation(float t)>;
