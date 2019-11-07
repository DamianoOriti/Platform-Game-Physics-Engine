#pragma once

#include <functional>
#include "vector_2.h"
#include "box_shape.h"
#include "circle_shape.h"
#include "chain_shape.h"
#include "capsule_shape.h"

class Body
{
	friend class Physics_Engine;
	friend class BinaryTree;

public:
	struct Collision;
	enum Type;

	float friction_;
	float bouncing_;
	Vector2f velocity_;

	Body(Type type, const Vector2f& position, Shape* shape, std::function<void(Collision& collision)> collision_callback, void* entity);
	~Body();

	void apply_impulse(const Vector2f& impulse);

	Type get_type() const;
	const Vector2f& get_position() const;
	const Shape* get_shape() const;
	Shape* get_shape();
	void* get_entiry();

private:
	Type type_;

	Shape* shape_;

	Vector2f position_;
	Vector2f impulse_;

	float min_x_;
	float max_x_;

	std::function<void(Collision& collision)> collision_callback_;

	void* entity_;
};

struct Body::Collision
{
	Vector2f normal;
	float distance;
	Body* collider_body;
};

enum Body::Type
{
	DYNAMIC,
	STATIC,
	SENSOR
};