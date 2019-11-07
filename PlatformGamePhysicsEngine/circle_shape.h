#pragma once

#include "shape.h"

class Circle_Shape :public Shape
{
	friend class Physics_Engine;

public:
	Circle_Shape(float radius);

	float get_radius() const;

private:
	float radius_;
};
