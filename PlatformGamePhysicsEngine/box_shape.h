#pragma once

#include "shape.h"

class Box_Shape :public Shape
{
	friend class Physics_Engine;

public:
	Box_Shape(float half_width, float half_height);

	float get_half_width() const;
	float get_half_height() const;

private:
	float half_width_;
	float half_height_;
};
