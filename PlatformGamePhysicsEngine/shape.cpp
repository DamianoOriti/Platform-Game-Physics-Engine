#include "shape.h"

Shape::Shape(Type type, float min_x, float max_x) :
	type_(type),
	min_x_(min_x),
	max_x_(max_x)
{
}

Shape::Type Shape::get_type() const
{
	return type_;
}

float Shape::get_min_x() const
{
	return min_x_;
}

float Shape::get_max_x() const
{
	return max_x_;
}
