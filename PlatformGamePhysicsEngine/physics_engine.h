#pragma once

#include "binary_tree.h"

class Physics_Engine
{
public:
	Physics_Engine(const Vector2f& gravity);

	void update(float delta_time);

	void add_body(Body* body);
	void remove_body(Body* body);
	void move_body(Body* body, const Vector2f& delta_position);

private:
	Vector2f gravity_;

	BinaryTree binary_tree_;

	std::vector<BinaryTree::Ball*> dynamic_body_balls_;
	std::vector<BinaryTree::Ball*> static_body_balls_;

	static bool fast_detect_collision(Body* dynamic_body, Body* collider_body);
	static void detect_and_solve_collision(Body* dynamic_body, std::vector<Body*>& other_bodies, float delta_time);
	static void detect_and_solve_circle_box_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction);
	static void detect_and_solve_circle_circle_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction);
	static void detect_and_solve_circle_chain_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction);
	static void detect_and_solve_capsule_box_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction);
	static void detect_and_solve_capsule_circle_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction);
	static void detect_and_solve_capsule_chain_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction);
};
