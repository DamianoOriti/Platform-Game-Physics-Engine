#pragma once

#include <stdexcept>

class Shape
{
	friend class Physics_Engine;

public:
	enum Type;

	Shape(Type type, float min_x, float max_x);
	virtual ~Shape() {}

	Type get_type() const;
	float get_min_x() const;
	float get_max_x() const;

private:
	Type type_;

	float min_x_;
	float max_x_;
};

enum Shape::Type
{
	BOX,
	CIRCLE,
	CAPSULE,
	CHAIN
};
