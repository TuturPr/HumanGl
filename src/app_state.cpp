#include "headers/app_state.hpp"
#include "headers/Camera.hpp"

// Camera and input state
bool firstMouse = true;
float lastX = 1280.0f / 2.0;
float lastY = 720.0 / 2.0;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Application state
bool mouseControlEnabled = false;
bool vertexMode = false;
bool keyPressed_V = false;
bool keyPressed_M = false;

// Window settings
float windowWidth = 1280;
float windowHeight = 720;
const float BASE_ASPECT_RATIO = 1280.0f / 720.0f;

// Camera instance
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
