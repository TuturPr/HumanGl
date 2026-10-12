#include "app/Application.hpp"

#include "geometry/RoundedBox.hpp"

#include <algorithm>

namespace {

constexpr float UNIT_SHAPE_ROUNDING = 0.2f;
constexpr int UNIT_SHAPE_SEGMENTS = 6;
constexpr float ORBIT_DEGREES_PER_PIXEL = 0.3f;
constexpr float ZOOM_STEP = 0.9f;
constexpr double MAX_FRAME_SECONDS = 0.1;	// avoids a jump after a stall (window drag...)

} // namespace

Application::Application(const std::string& shaderDirectory)
	: window_(1280, 720, "HumanGL"),
	  unitShape_(makeRoundedBox(UNIT_SHAPE_ROUNDING, UNIT_SHAPE_SEGMENTS)),
	  character_(unitShape_.positions),
	  renderer_(shaderDirectory, unitShape_),
	  animations_(AnimationLibrary::createDefault()),
	  imgui_(window_.handle()),
	  panel_(animations_, animator_, character_, renderer_, camera_)
{
	animator_.play(animations_.at(0));
}

void Application::run()
{
	double previousTime = window_.time();
	while (!window_.shouldClose())
	{
		window_.pollEvents();
		handleInput();

		const double now = window_.time();
		animator_.update(static_cast<float>(std::min(now - previousTime, MAX_FRAME_SECONDS)));
		previousTime = now;

		int width = 0;
		int height = 0;
		window_.framebufferSize(width, height);
		renderer_.render(character_, animator_.pose(), camera_, width, height);

		imgui_.beginFrame();
		panel_.draw();
		imgui_.endFrame();
		window_.swapBuffers();
	}
}

void Application::handleInput()
{
	if (!imgui_.wantsKeyboard())
		handleKeyboard();
	if (!imgui_.wantsMouse())
		handleMouse();
}

void Application::handleKeyboard()
{
	for (int key : window_.pressedKeys())
	{
		if (key == GLFW_KEY_ESCAPE)
			window_.close();
		else if (key == GLFW_KEY_SPACE)
			animator_.setPaused(!animator_.paused());
		else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9)
		{
			const std::size_t choice = static_cast<std::size_t>(key - GLFW_KEY_1);
			if (choice < animations_.size())
				animator_.play(animations_.at(choice));
		}
	}
}

void Application::handleMouse()
{
	if (window_.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
	{
		double dx = 0.0;
		double dy = 0.0;
		window_.cursorDelta(dx, dy);
		camera_.orbit(static_cast<float>(-dx) * ORBIT_DEGREES_PER_PIXEL,
					  static_cast<float>(dy) * ORBIT_DEGREES_PER_PIXEL);
	}
	if (window_.scrollDelta() != 0.0)
		camera_.zoom(window_.scrollDelta() > 0.0 ? ZOOM_STEP : 1.0f / ZOOM_STEP);
}
