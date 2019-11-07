#include "box_shape.h"

Box_Shape::Box_Shape(float half_width, float half_height) :
	Shape(Type::BOX, -half_width, half_width),
	half_width_(half_width),
	half_height_(half_height)
{
	if (half_width <= 0.0f || half_height <= 0.0f)
	{
		throw std::runtime_error("half width and/or half height are negative!");
	}
}

float Box_Shape::get_half_width() const
{
	return half_width_;
}

float Box_Shape::get_half_height() const
{
	return half_height_;
}
