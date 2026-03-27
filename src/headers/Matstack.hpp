#pragma once
#include <vector>
#include <glm/glm.hpp>

class Matstack
{

	public:
		Matstack();
		std::vector<glm::mat4> stack;
		void pushMatrix();
		void popMatrix();
		void rotate(float angle, const glm::vec3& axis);
		void translate(const glm::vec3& offset);
		void scale(const glm::vec3& factors);
		~Matstack();
};

