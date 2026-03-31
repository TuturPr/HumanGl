#pragma once
#include "Matstack.hpp"
#include "Mesh.hpp"
#include "Animation.hpp"

enum class AnimState { IDLE, WALK, JUMP, DISCO };

class Model {
public:
    Model();
    ~Model();

    Mesh mesh;
    Matstack matstack;
    float animationTime = 0.0f;

    AnimState   currentState = AnimState::IDLE;
    AnimationFunc currentAnim;

    void setAnimation(AnimationFunc func);
    void updateAnimation(float deltaTime);
    void DrawModel(Shader &shader);

private:
    Animation state;

    void DrawCube(Shader &shader);
    void DrawHead(Shader &shader);
    void DrawTorso(Shader &shader);
    void DrawArm(Shader &shader, char side);
    void DrawForearm(Shader &shader, char side);
    void DrawTopLeg(Shader &shader, char side);
    void DrawBottomLeg(Shader &shader, char side);
};
