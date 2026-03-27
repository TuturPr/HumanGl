#include "headers/ObjLoader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

std::map<std::string, std::string> ObjLoader::loadMtl(const std::string& mtlPath) {
	std::map<std::string, std::string> textures;
	std::ifstream file(mtlPath);

	if (!file.is_open()) {
		std::cerr << "Warning: MTL file not found (" << mtlPath << ")\n";
		return textures;
	}

	const fs::path mtlDir = fs::path(mtlPath).parent_path();
	std::string line, currentMaterial;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "newmtl") {
			iss >> currentMaterial;
		} else if (prefix == "map_Kd" && !currentMaterial.empty()) {
			std::string texPath;
			std::getline(iss, texPath);

			// Trim whitespace
			auto start = texPath.find_first_not_of(" \t\r\n");
			auto end   = texPath.find_last_not_of(" \t\r\n");
			if (start == std::string::npos) continue;
			texPath = texPath.substr(start, end - start + 1);

			fs::path fullPath = fs::path(texPath).is_absolute() ? fs::path(texPath) : mtlDir / texPath;

			// Fallback: if absolute path doesn't exist, look next to the MTL
			if (fullPath.is_absolute() && !fs::exists(fullPath))
				fullPath = mtlDir / fullPath.filename();

			fullPath = fs::absolute(fullPath.lexically_normal());
			textures[currentMaterial] = fullPath.string();
			std::cout << "Material: " << currentMaterial << " -> " << fullPath << "\n";
		}
	}

	return textures;
}

Mesh ObjLoader::load(const std::string& filepath) {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
	std::map<std::string, unsigned int> cache;
	unsigned int faceCount = 0;

	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Error: Cannot open OBJ file (" << filepath << ")\n";
		return Mesh(vertices, indices, {});
	}

	const std::string dir = fs::path(filepath).parent_path().string() + "/";
	std::map<std::string, std::string> materialTextures;
	std::string line;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "mtllib") {
			std::string mtlFile; iss >> mtlFile;
			materialTextures = loadMtl(dir + mtlFile);
		} else if (prefix == "v") {
			glm::vec3 p; iss >> p.x >> p.y >> p.z; positions.push_back(p);
		} else if (prefix == "vt") {
			glm::vec2 uv; iss >> uv.x >> uv.y; uvs.push_back(uv);
		} else if (prefix == "vn") {
			glm::vec3 n; iss >> n.x >> n.y >> n.z; normals.push_back(n);
		} else if (prefix == "f") {
			std::vector<std::string> face;
			for (std::string v; iss >> v;) face.push_back(v);

			// Generate a unique gray shade for this face
			float grayValue = 0.3f + 0.4f * ((faceCount * 73) % 256) / 256.0f; // Value between 0.3 and 0.7
			glm::vec3 faceColor(grayValue);

			for (size_t i = 1; i + 1 < face.size(); ++i) {
				for (const auto& token : { face[0], face[i], face[i + 1] }) {
					if (cache.count(token)) { indices.push_back(cache[token]); continue; }

					std::istringstream ts(token);
					std::string part;
					std::vector<int> idx;
					while (std::getline(ts, part, '/'))
						idx.push_back(part.empty() ? 0 : std::stoi(part));

					auto get = [](auto& vec, int i, auto def) {
						return (i > 0 && i <= (int)vec.size()) ? vec[i - 1] : def;
					};

					Vertex v;
					v.Position  = get(positions, idx.size() > 0 ? idx[0] : 0, glm::vec3(0));
					v.TexCoords = get(uvs,       idx.size() > 1 ? idx[1] : 0, glm::vec2(0));
					v.Normal    = get(normals,    idx.size() > 2 ? idx[2] : 0, glm::vec3(0, 0, 1));
					v.Color     = faceColor;

					cache[token] = vertices.size();
					indices.push_back(vertices.size());
					vertices.push_back(v);
				}
			}
			faceCount++;
		}
	}

	std::vector<MeshTexture> textures;
	for (const auto& [mat, path] : materialTextures)
		textures.push_back({0, "diffuse", path});

	return Mesh(vertices, indices, textures);
}
