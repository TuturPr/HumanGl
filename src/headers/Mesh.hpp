#pragma once
#include <vector>
#include "ShaderClass.hpp"
#include "Structs.hpp"
#include <glm/glm.hpp>

class Mesh {
	public:
		std::vector<Vertex>       vertices;
		std::vector<unsigned int> indices;
		std::vector<MeshTexture>      textures;

		Mesh();
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures);
		void Draw(Shader &shader);
		void addTexture(unsigned int textureId, const std::string& type);
	void setTextureId(int index, unsigned int textureId);

	private:
		unsigned int VAO, VBO, EBO;
		void setupMesh();
};
