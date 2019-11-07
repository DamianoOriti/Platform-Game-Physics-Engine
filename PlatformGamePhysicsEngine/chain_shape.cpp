#include "chain_shape.h"

Chain_Shape::Chain_Shape(const std::vector<Vector2f>& vertices) :
	Shape(Shape::Type::CHAIN, vertices[0].x, vertices[vertices.size() - 1].x),
	vertices_(vertices)
{
	if (vertices[0].y != vertices[vertices.size() - 1].y)
	{
		throw std::runtime_error("first and last vertices have not the same y coordinate!");
	}

	for (size_t i = 1; i < vertices.size(); i++)
	{
		if (vertices[i].x < vertices[i - 1].x)
		{
			throw std::runtime_error("a vertex is on the left of its predecessor!");
		}
	}
}

const std::vector<Vector2f>& Chain_Shape::get_vertices() const
{
	return vertices_;
}
