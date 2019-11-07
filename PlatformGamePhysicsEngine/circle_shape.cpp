#include "circle_shape.h"

Circle_Shape::Circle_Shape(float radius) :
	Shape(Shape::Type::CIRCLE, -radius, radius),
	radius_(radius)
{
	if (radius <= 0.0f)
	{
		throw std::runtime_error("the radius is negative!");
	}
}

float Circle_Shape::get_radius() const
{
	return radius_;
}
