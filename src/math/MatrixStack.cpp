#include "math/MatrixStack.hpp"

#include <stdexcept>

MatrixStack::MatrixStack() : stack_{Mat4::identity()} {}

void MatrixStack::push()
{
	stack_.push_back(stack_.back());
}

void MatrixStack::pop()
{
	if (stack_.size() <= 1)
		throw std::logic_error("MatrixStack::pop: cannot pop the base matrix");
	stack_.pop_back();
}

const Mat4& MatrixStack::top() const
{
	return stack_.back();
}

std::size_t MatrixStack::depth() const
{
	return stack_.size();
}

void MatrixStack::loadIdentity()
{
	stack_.back() = Mat4::identity();
}

void MatrixStack::multiply(const Mat4& matrix)
{
	stack_.back() = stack_.back() * matrix;
}

void MatrixStack::translate(const Vec3& offset)
{
	multiply(Mat4::translation(offset));
}

void MatrixStack::rotate(float degrees, const Vec3& axis)
{
	multiply(Mat4::rotation(degrees, axis));
}

void MatrixStack::rotateX(float degrees)
{
	multiply(Mat4::rotationX(degrees));
}

void MatrixStack::rotateY(float degrees)
{
	multiply(Mat4::rotationY(degrees));
}

void MatrixStack::rotateZ(float degrees)
{
	multiply(Mat4::rotationZ(degrees));
}

void MatrixStack::scale(const Vec3& factors)
{
	multiply(Mat4::scaling(factors));
}
