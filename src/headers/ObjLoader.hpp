#pragma once

#include "Mesh.hpp"
#include <string>
#include <map>

class ObjLoader {
public:
	static Mesh load(const std::string& filepath);

private:
	static std::map<std::string, std::string> loadMtl(const std::string& mtlPath);
};
