#pragma once
#include <glm/glm.hpp>
#include <string>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Color;
};

struct MeshTexture {
	unsigned int id;
	std::string type;
	std::string path;
};
