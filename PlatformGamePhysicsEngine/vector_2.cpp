#include "vector_2.h"

template<>
Vector2<float>::Vector2() :
	x(0.0f),
	y(0.0f)
{
}

template<>
float Vector2<float>::compute_length() const
{
	return sqrtf(this->x * this->x + this->y * this->y);
}

template<>
Vector2<float>& Vector2<float>::normalize()
{
	float length = this->compute_length();

	this->x /= length;
	this->y /= length;

	return *this;
}

template<>
Vector2<float> Vector2<float>::normalized() const
{
	Vector2f result;

	float length = this->compute_length();

	result.x = this->x / length;
	result.y = this->y / length;

	return result;
}
