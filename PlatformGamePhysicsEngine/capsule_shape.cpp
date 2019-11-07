#include "capsule_shape.h"

Capsule_Shape::Capsule_Shape(float radius, float distance) :
	Shape(Shape::Type::CAPSULE, -radius, radius),
	radius_(radius),
	distance_(distance)
{
	if (radius <= 0.0f || distance < 0.0f)
	{
		throw std::runtime_error("the radius and/or the distance are negative!");
	}
}

void Capsule_Shape::set_distance(float distance)
{
	if (distance < 0.0f)
	{
		return;
	}

	distance_ = distance;
}

float Capsule_Shape::get_radius() const
{
	return radius_;
}

float Capsule_Shape::get_distance() const
{
	return distance_;
}
