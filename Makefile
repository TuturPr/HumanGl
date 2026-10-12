NAME		= humangl
TEST_NAME	= tests/run_tests

CXX			= c++
CC			= cc
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -O2 -MMD -MP
CFLAGS		= -Wall -Wextra -Werror -O2
INCLUDES	= -I src -I includes -I includes/imgui-1.92.6 -I includes/imgui-1.92.6/backends
DEFINES		= -D SHADER_DIR=\"$(CURDIR)/src/shaders/\"
LDLIBS		= -lglfw -ldl -pthread -lm

BUILD		= .tmp

# ---------------------------------------------------------------- sources ---

# Modules without any OpenGL dependency (also linked into the unit tests).
MATH_SRC		= src/math/Mat4.cpp src/math/MatrixStack.cpp
GEOMETRY_SRC	= src/geometry/RoundedBox.cpp
SKELETON_SRC	= src/skeleton/BodyPart.cpp src/skeleton/BodyDimensions.cpp src/skeleton/Pose.cpp \
				  src/skeleton/JointLimits.cpp src/skeleton/HumanModel.cpp src/skeleton/Character.cpp
ANIMATION_SRC	= src/animation/Animator.cpp src/animation/AnimationLibrary.cpp \
				  src/animation/clips/Idle.cpp src/animation/clips/Walk.cpp src/animation/clips/Jump.cpp
CAMERA_SRC		= src/render/OrbitCamera.cpp src/render/PlanarShadow.cpp src/render/BodyPalette.cpp
CORE_SRC		= $(MATH_SRC) $(GEOMETRY_SRC) $(SKELETON_SRC) $(ANIMATION_SRC) $(CAMERA_SRC)

# OpenGL / window / application.
RENDER_SRC		= src/render/GpuMesh.cpp src/render/ShaderProgram.cpp src/render/PartDrawers.cpp \
				  src/render/SceneRenderer.cpp
APP_SRC			= src/platform/Window.cpp src/app/Application.cpp src/main.cpp
GLAD_SRC		= libs/glad.c

SRC				= $(CORE_SRC) $(RENDER_SRC) $(APP_SRC)

TEST_SRC		= tests/test_main.cpp \
				  tests/math/test_Vec3.cpp tests/math/test_Mat4.cpp tests/math/test_MatrixStack.cpp

OBJ				= $(SRC:%.cpp=$(BUILD)/%.o) $(GLAD_SRC:%.c=$(BUILD)/%.o)
TEST_OBJ		= $(CORE_SRC:%.cpp=$(BUILD)/%.o) $(TEST_SRC:%.cpp=$(BUILD)/%.o)

# ------------------------------------------------------------------ rules ---

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDLIBS) -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $(TEST_OBJ) -lgtest -pthread -o $@

clean:
	rm -rf $(BUILD)

fclean: clean
	rm -f $(NAME) $(TEST_NAME)

re: fclean all

-include $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

.PHONY: all test clean fclean re
