#include "body.h"

Body::Body(Type type, const Vector2f& position, Shape* shape, std::function<void(Collision& collision)> collision_callback, void* entity) :
	type_(type),
	collision_callback_(collision_callback),
	entity_(entity),
	shape_(shape),
	position_(position),
	impulse_(0.0f, 0.0f),
	velocity_(0.0f, 0.0f),
	min_x_(position.x + shape->get_min_x()),
	max_x_(position.x + shape->get_max_x())
{
}

Body::~Body()
{
	delete shape_;
}

void Body::apply_impulse(const Vector2f& impulse)
{
	impulse_ += impulse;
}

Body::Type Body::get_type() const
{
	return type_;
}

const Vector2f& Body::get_position() const
{
	return position_;
}

const Shape* Body::get_shape() const
{
	return shape_;
}

Shape* Body::get_shape()
{
	return shape_;
}

void* Body::get_entiry()
{
	return entity_;
}
