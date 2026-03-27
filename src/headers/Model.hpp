#pragma once
#include "Mesh.hpp"
#include "ShaderClass.hpp"

class Model
{
	private:


	public:
		Mesh mesh;
		void DrawCube(Shader &shader, glm::mat4 model);
		Model();
		~Model();
};
