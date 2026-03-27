#include "headers/Matstack.hpp"
#include <glm/gtc/matrix_transform.hpp>

Matstack::Matstack()
{
	// Initialize with identity matrix
	stack.push_back(glm::mat4(1.0f));
}

Matstack::~Matstack()
{
}

void Matstack::pushMatrix()
{
	//Save latest matrix state
	stack.push_back(stack.back());
}

void Matstack::popMatrix()
{
	if (!stack.empty()) {
		stack.pop_back();
	}
}
void Matstack::rotate(float angle, const glm::vec3& axis)
{
	if (!stack.empty()) {
		stack.back() = glm::rotate(stack.back(), glm::radians(angle), axis);
	}
}

void Matstack::translate(const glm::vec3& offset)
{
	if (!stack.empty()) {
		stack.back() = glm::translate(stack.back(), offset);
	}
}

void Matstack::scale(const glm::vec3& factors)
{
	if (!stack.empty()) {
		stack.back() = glm::scale(stack.back(), factors);
	}
}
