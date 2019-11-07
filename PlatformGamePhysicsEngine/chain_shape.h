#pragma once

#include <vector>
#include "vector_2.h"
#include "shape.h"

class Chain_Shape :public Shape
{
	friend class Physics_Engine;

public:
	Chain_Shape(const std::vector<Vector2f>& vertices);

	const std::vector<Vector2f>& get_vertices() const;

private:
	std::vector<Vector2f> vertices_;
};
