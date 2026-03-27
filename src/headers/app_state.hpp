#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Camera.hpp"

// Camera and input state
extern bool firstMouse;
extern float lastX;
extern float lastY;
extern float deltaTime;
extern float lastFrame;

// Application state
extern bool mouseControlEnabled;
extern bool vertexMode;
extern bool keyPressed_V;
extern bool keyPressed_M;

// Window settings
extern float windowWidth;
extern float windowHeight;
extern const float BASE_ASPECT_RATIO;

// Camera instance
extern Camera camera;
