#pragma once

#include "shape.h"

class Capsule_Shape :public Shape
{
	friend class Physics_Engine;

public:
	Capsule_Shape(float radius, float distance);

	void set_distance(float distance);

	float get_radius() const;
	float get_distance() const;

private:
	float radius_;
	float distance_;
};
