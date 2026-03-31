#include "headers/Matstack.hpp"
#include "headers/Model.hpp"
#include "headers/ObjLoader.hpp"
#include "headers/Animations.hpp"
#include <cmath>

Model::Model()
{
    this->mesh = ObjLoader::load("/home/arthur/Documents/HumanGl/src/assets/cube.obj");
    this->animationTime = 0.0f;
    this->currentAnim = Animations::idle();
}

Model::~Model() {}

void Model::setAnimation(AnimationFunc func)
{
    currentAnim = func;
    animationTime = 0.0f;
}

void Model::updateAnimation(float deltaTime)
{
    animationTime += deltaTime;
    if (currentAnim)
        state = currentAnim(animationTime);
}

// 1x1x1 à l'origine — utilise le top du matstack
void Model::DrawCube(Shader &shader)
{
    shader.setMat4("model", matstack.stack.back());
    this->mesh.Draw(shader);
}

void Model::DrawModel(Shader &shader)
{
    // Torso = racine, tout est enfant
    matstack.pushMatrix();
    matstack.translate(glm::vec3(0.0f, state.bodyOffsetY, 0.0f));
    matstack.rotate(state.torsoAngle, glm::vec3(0, 1, 0));

    this->DrawTorso(shader);
    this->DrawHead(shader);
    this->DrawArm(shader, 'L');
    this->DrawArm(shader, 'R');
    this->DrawTopLeg(shader, 'L');
    this->DrawTopLeg(shader, 'R');

    matstack.popMatrix();
}

void Model::DrawHead(Shader &shader)
{
    matstack.pushMatrix();
    matstack.translate(glm::vec3(0.0f, 1.5f, 0.0f));
    matstack.rotate(state.headAngle, glm::vec3(0, 1, 0));
    matstack.scale(glm::vec3(0.5f, 0.5f, 0.5f));
    this->DrawCube(shader);
    matstack.popMatrix();
}

void Model::DrawTorso(Shader &shader)
{
    matstack.pushMatrix();
    matstack.scale(glm::vec3(0.5f, 1.0f, 0.3f));
    this->DrawCube(shader);
    matstack.popMatrix();
}

void Model::DrawArm(Shader &shader, char side)
{
    float sign  = (side == 'L') ? -1.0f : 1.0f;
    float angle = (side == 'L') ? state.armAngle_L : state.armAngle_R;

    matstack.pushMatrix();
    matstack.translate(glm::vec3(sign * 0.70f, 0.7f, 0.0f));
    matstack.rotate(angle, glm::vec3(1, 0, 0));  // pivot = épaule
    matstack.translate(glm::vec3(0.0f, -0.3f, 0.0f));
    matstack.scale(glm::vec3(0.2f, 0.6f, 0.3f));
    this->DrawCube(shader);
    this->DrawForearm(shader, side);
    matstack.popMatrix();
}

void Model::DrawForearm(Shader &shader, char side)
{
    float angle = (side == 'L') ? state.forearmAngle_L : state.forearmAngle_R;

    matstack.pushMatrix();
    matstack.translate(glm::vec3(0.0f, -0.9f, 0.0f));  // pivot = coude
    matstack.rotate(angle, glm::vec3(1, 0, 0));
    matstack.translate(glm::vec3(0.0f, -0.3f, 0.0f));
    matstack.scale(glm::vec3(1.0f, 0.6f, 1.0f));
    glPolygonOffset(1.0f, 1.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    this->DrawCube(shader);
    glDisable(GL_POLYGON_OFFSET_FILL);
    matstack.popMatrix();
}

void Model::DrawTopLeg(Shader &shader, char side)
{
    float sign  = (side == 'L') ? -1.0f : 1.0f;
    float angle = (side == 'L') ? state.legAngle_L : state.legAngle_R;

    matstack.pushMatrix();
    matstack.translate(glm::vec3(sign * 0.25f, -0.9f, 0.0f));
    matstack.rotate(angle, glm::vec3(1, 0, 0));  // pivot = hanche
    matstack.translate(glm::vec3(0.0f, -0.6f, 0.0f));
    matstack.scale(glm::vec3(0.25f, 0.6f, 0.3f));
	glPolygonOffset(1.0f, 1.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    this->DrawCube(shader);
	glDisable(GL_POLYGON_OFFSET_FILL);
    this->DrawBottomLeg(shader, side);
    matstack.popMatrix();
}

void Model::DrawBottomLeg(Shader &shader, char side)
{
    float angle = (side == 'L') ? state.shinAngle_L : state.shinAngle_R;

    matstack.pushMatrix();
    matstack.translate(glm::vec3(0.0f, -0.9f, 0.0f));  // pivot = genou
    matstack.rotate(angle, glm::vec3(1, 0, 0));
    matstack.translate(glm::vec3(0.0f, -0.3f, 0.0f));
    matstack.scale(glm::vec3(1.0f, 0.6f, 1.0f));
	glPolygonOffset(2.0f, 2.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    this->DrawCube(shader);
	glDisable(GL_POLYGON_OFFSET_FILL);
    matstack.popMatrix();
}
