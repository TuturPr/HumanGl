#pragma once

#include "math/Mat4.hpp"

#include <vector>

// Replacement for the deprecated glPushMatrix / glPopMatrix / glTranslate ...
// Every transformation is post-multiplied into the current (top) matrix, so
// calls read in the same order as the hierarchy: parent first, child after.
class MatrixStack
{
public:
	MatrixStack();

	void push();
	void pop();	// throws std::logic_error when popping the base matrix

	const Mat4& top() const;
	std::size_t depth() const;

	void loadIdentity();
	void multiply(const Mat4& matrix);
	void translate(const Vec3& offset);
	void rotate(float degrees, const Vec3& axis);
	void rotateX(float degrees);
	void rotateY(float degrees);
	void rotateZ(float degrees);
	void scale(const Vec3& factors);

private:
	std::vector<Mat4> stack_;
};
