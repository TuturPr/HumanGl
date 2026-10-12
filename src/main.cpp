#include "app/Application.hpp"

#include <exception>
#include <iostream>

#ifndef SHADER_DIR
# define SHADER_DIR "src/shaders/"
#endif

int main()
{
	try
	{
		Application application(SHADER_DIR);
		application.run();
	}
	catch (const std::exception& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}
	return 0;
}
