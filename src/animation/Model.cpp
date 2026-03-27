#include "headers/Matstack.hpp"
#include "headers/Model.hpp"
#include "headers/ObjLoader.hpp"

Model::Model()
{
	this->mesh = ObjLoader::load("/home/arthur/Documents/HumanGl/src/assets/cube.obj");
}

Model::~Model()
{
}

void Model::DrawCube(Shader &shader, glm::mat4 model)
{
	shader.setMat4("model", model);
	this->mesh.Draw(shader);
}

