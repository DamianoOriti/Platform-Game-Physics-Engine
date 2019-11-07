#include "physics_engine.h"

Physics_Engine::Physics_Engine(const Vector2f& gravity) :
	gravity_(gravity),
	binary_tree_(20.0f)
{
}

void Physics_Engine::update(float delta_time)
{
	// update velocity and position of dynamic bodies.
	// update binary tree.
	for each (auto ball in dynamic_body_balls_)
	{
		Body* body = ball->body;

		// add gravity effect to impulse
		body->impulse_ += gravity_ * delta_time;

		// update velocity
		body->velocity_ += body->impulse_;

		// update position
		body->position_ += body->velocity_ * delta_time;

		// clear impulse
		body->impulse_ = Vector2f(0.0f, 0.0f);

		// update min_x and max_x
		body->min_x_ = body->position_.x + body->shape_->get_min_x();
		body->max_x_ = body->position_.x + body->shape_->get_max_x();


		// update binary tree
		binary_tree_.update(ball);
	}

	// for each dynamic body detect collisions and solve them
	for each (auto ball in dynamic_body_balls_)
	{
		detect_and_solve_collision(ball->body, ball->bodies, delta_time);
	}
}

void Physics_Engine::add_body(Body* body)
{
	// create a new ball with the body
	BinaryTree::Ball* ball = new BinaryTree::Ball(body);

	// add the ball to the appropriate vector
	if (body->type_ == Body::Type::DYNAMIC)
	{
		dynamic_body_balls_.push_back(ball);
	}
	else
	{
		static_body_balls_.push_back(ball);
	}

	// add the ball to the binary tree
	binary_tree_.add_ball(ball);
}

void Physics_Engine::remove_body(Body* body)
{
	auto& balls = body->type_ == Body::Type::DYNAMIC ? dynamic_body_balls_ : static_body_balls_;

	for (auto it = balls.begin(); it != balls.end(); it++)
	{
		if ((*it)->body == body)
		{
			binary_tree_.remove_ball(*it);

			balls.erase(it);

			delete body;
			return;
		}
	}
}

void Physics_Engine::move_body(Body* body, const Vector2f& delta_position)
{
	body->position_.y += delta_position.y;

	if (delta_position.x != 0.0f)
	{
		auto& balls = body->type_ == Body::Type::DYNAMIC ? dynamic_body_balls_ : static_body_balls_;

		for (auto it = balls.begin(); it != balls.end(); it++)
		{
			if ((*it)->body == body)
			{
				binary_tree_.remove_ball(*it);

				body->position_.x += delta_position.x;

				binary_tree_.add_ball(*it);

				return;
			}
		}
	}
}

bool Physics_Engine::fast_detect_collision(Body* dynamic_body, Body* collider_body)
{
	return (dynamic_body->min_x_ < collider_body->max_x_) && (dynamic_body->max_x_ > collider_body->min_x_);
}

void Physics_Engine::detect_and_solve_collision(Body* dynamic_body, std::vector<Body*>& other_bodies, float deltaTime)
{
	Vector2f position_correction(0.0f, 0.0f);
	Vector2f velocity_correction(0.0f, 0.0f);
	for each (auto body in other_bodies)
	{
		if (!fast_detect_collision(dynamic_body, body))
		{
			continue;
		}

		if (dynamic_body->shape_->type_ == Shape::Type::CIRCLE)
		{
			if (body->shape_->type_ == Shape::Type::BOX)
			{
				detect_and_solve_circle_box_collision(dynamic_body, body, position_correction, velocity_correction);
			}
			else if (body->shape_->type_ == Shape::Type::CIRCLE)
			{
				detect_and_solve_circle_circle_collision(dynamic_body, body, position_correction, velocity_correction);
			}
			else if (body->shape_->type_ == Shape::Type::CHAIN)
			{
				detect_and_solve_circle_chain_collision(dynamic_body, body, position_correction, velocity_correction);
			}
		}
		else if (dynamic_body->shape_->type_ == Shape::Type::CAPSULE)
		{
			if (body->shape_->type_ == Shape::Type::BOX)
			{
				detect_and_solve_capsule_box_collision(dynamic_body, body, position_correction, velocity_correction);
			}
			else if (body->shape_->type_ == Shape::Type::CIRCLE)
			{
				detect_and_solve_capsule_circle_collision(dynamic_body, body, position_correction, velocity_correction);
			}
			else if (body->shape_->type_ == Shape::Type::CHAIN)
			{
				detect_and_solve_capsule_chain_collision(dynamic_body, body, position_correction, velocity_correction);
			}
		}
	}

	dynamic_body->position_ += position_correction;
	dynamic_body->velocity_ += velocity_correction;
}

void Physics_Engine::detect_and_solve_circle_box_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction)
{
	Circle_Shape* dynamic_body_shape = static_cast<Circle_Shape*>(dynamic_body->shape_);
	Box_Shape* other_body_shape = static_cast<Box_Shape*>(other_body->shape_);

	Vector2f delta_position = dynamic_body->position_ - other_body->position_;

	Vector2f v = delta_position;
	v.x = clamp<float>(v.x, -other_body_shape->half_width_, other_body_shape->half_width_);
	v.y = clamp<float>(v.y, -other_body_shape->half_height_, other_body_shape->half_height_);

	Vector2f n = delta_position - v;

	float distance = n.compute_length() - dynamic_body_shape->radius_;
	if (distance >= 0.0f)
	{
		return;
	}

	if (other_body->type_ == Body::Type::STATIC)
	{
		position_correction -= n.normalize() * distance;

		Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
		velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

		Vector2f p = n.ortho();
		velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
	}

	if (dynamic_body->collision_callback_ != nullptr)
	{
		Body::Collision collision;
		collision.collider_body = other_body;
		collision.distance = distance;
		collision.normal = n;

		dynamic_body->collision_callback_(collision);
	}
}

void Physics_Engine::detect_and_solve_circle_circle_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction)
{
	Circle_Shape* dynamic_body_shape = static_cast<Circle_Shape*>(dynamic_body->shape_);
	Circle_Shape* other_body_shape = static_cast<Circle_Shape*>(other_body->shape_);

	Vector2f delta_position = dynamic_body->position_ - other_body->position_;

	float distance = delta_position.compute_length() - (dynamic_body_shape->radius_ + other_body_shape->radius_);
	if (distance >= 0.0f)
	{
		return;
	}

	Vector2f n = delta_position.normalized();

	if (other_body->type_ == Body::Type::STATIC)
	{
		position_correction -= n * distance;

		Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
		velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

		Vector2f p = n.ortho();
		velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
	}

	if (dynamic_body->collision_callback_ != nullptr)
	{
		Body::Collision collision;
		collision.collider_body = other_body;
		collision.distance = distance;
		collision.normal = n;

		dynamic_body->collision_callback_(collision);
	}
}

void Physics_Engine::detect_and_solve_circle_chain_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction)
{
	Circle_Shape* dynamic_body_shape = static_cast<Circle_Shape*>(dynamic_body->shape_);
	Chain_Shape* other_body_shape = static_cast<Chain_Shape*>(other_body->shape_);

	const std::vector<Vector2f>& vertices = other_body_shape->vertices_;

	bool has_collided = false;
	Vector2f p_c(0.0f, 0.0f);
	for (size_t i = 0; i < vertices.size() - 2; i += 2)
	{
		const Vector2f v0(vertices[i].x, vertices[0].y);
		const Vector2f& v1 = vertices[i + 1];
		const Vector2f& v2 = vertices[i + 2];

		Vector2f c = (v0 + v2) / 2.0f;

		float half_width = v2.x - c.x;
		float half_height = v1.y - c.y;

		c += other_body->position_;

		Vector2f delta_position = dynamic_body->position_ - c;

		Vector2f v = delta_position;
		v.x = clamp<float>(delta_position.x, -half_width, half_width);
		v.y = clamp<float>(delta_position.y, -half_height, half_height);

		Vector2f n = delta_position - v;

		float distance = n.compute_length() - dynamic_body_shape->radius_;
		if (distance >= 0.0f)
		{
			if (n.x < 0.0f)
			{
				break;
			}

			continue;
		}

		p_c -= n.normalize() * distance;

		has_collided = true;
	}

	if (has_collided)
	{
		float distance = p_c.compute_length();
		Vector2f n = p_c.normalized();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction += p_c;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
}

void Physics_Engine::detect_and_solve_capsule_box_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction)
{
	Capsule_Shape* dynamic_body_shape = static_cast<Capsule_Shape*>(dynamic_body->shape_);
	Box_Shape* other_body_shape = static_cast<Box_Shape*>(other_body->shape_);

	if (dynamic_body->position_.y >= other_body->position_.y + other_body_shape->half_height_)
	{
		Vector2f delta_position = dynamic_body->position_ - other_body->position_;

		Vector2f v = delta_position;
		v.x = clamp<float>(delta_position.x, -other_body_shape->half_width_, other_body_shape->half_width_);
		v.y = clamp<float>(delta_position.y, -other_body_shape->half_height_, other_body_shape->half_height_);

		Vector2f n = delta_position - v;

		float distance = n.compute_length() - dynamic_body_shape->radius_;
		if (distance >= 0.0f)
		{
			return;
		}

		n.normalize();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction -= n.normalize() * distance;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
	else if (dynamic_body->position_.y + dynamic_body_shape->distance_ <= other_body->position_.y - other_body_shape->half_height_)
	{
		Vector2f delta_position = dynamic_body->position_ - other_body->position_;
		delta_position.y += dynamic_body_shape->distance_;

		Vector2f v = delta_position;
		v.x = clamp<float>(delta_position.x, -other_body_shape->half_width_, other_body_shape->half_width_);
		v.y = clamp<float>(delta_position.y, -other_body_shape->half_height_, other_body_shape->half_height_);

		Vector2f n = delta_position - v;

		float distance = n.compute_length() - dynamic_body_shape->radius_;
		if (distance >= 0.0f)
		{
			return;
		}

		n.normalize();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction -= n.normalize() * distance;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
	else
	{
		float delta_position_x = dynamic_body->position_.x - other_body->position_.x;

		float v_x = clamp<float>(delta_position_x, -other_body_shape->half_width_, other_body_shape->half_width_);

		Vector2f n(delta_position_x - v_x, 0.0f);

		float distance = fabs(n.x) - dynamic_body_shape->radius_;
		if (distance >= 0.0f)
		{
			return;
		}

		n.normalize();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction -= n * distance;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
}

void Physics_Engine::detect_and_solve_capsule_circle_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction)
{
	Capsule_Shape* dynamic_body_shape = static_cast<Capsule_Shape*>(dynamic_body->shape_);
	Circle_Shape* other_body_shape = static_cast<Circle_Shape*>(other_body->shape_);

	if (dynamic_body->position_.y >= other_body->position_.y)
	{
		Vector2f delta_position = dynamic_body->position_ - other_body->position_;

		float distance = delta_position.compute_length() - (dynamic_body_shape->radius_ + other_body_shape->radius_);
		if (distance >= 0.0f)
		{
			return;
		}

		Vector2f n = delta_position.normalized();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction -= n * distance;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
	else if (dynamic_body->position_.y + dynamic_body_shape->distance_ <= other_body->position_.y)
	{
		Vector2f delta_position = dynamic_body->position_ - other_body->position_;
		delta_position.y += dynamic_body_shape->distance_;

		float distance = delta_position.compute_length() - (dynamic_body_shape->radius_ + other_body_shape->radius_);
		if (distance >= 0.0f)
		{
			return;
		}

		Vector2f n = delta_position.normalized();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction -= n * distance;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
	else
	{
		float delta_position_x = dynamic_body->position_.x - other_body->position_.x;

		float v_x = clamp<float>(delta_position_x, -other_body_shape->radius_, other_body_shape->radius_);

		Vector2f n(delta_position_x - v_x, 0.0f);

		float distance = fabs(n.x) - dynamic_body_shape->radius_;
		if (distance >= 0.0f)
		{
			return;
		}

		n.normalize();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction -= n * distance;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
}

void Physics_Engine::detect_and_solve_capsule_chain_collision(Body* dynamic_body, Body* other_body, Vector2f& position_correction, Vector2f& velocity_correction)
{
	Capsule_Shape* dynamic_body_shape = static_cast<Capsule_Shape*>(dynamic_body->shape_);
	Chain_Shape* other_body_shape = static_cast<Chain_Shape*>(other_body->shape_);

	const std::vector<Vector2f>& vertices = other_body_shape->vertices_;

	bool has_collided = false;
	Vector2f p_c(0.0f, 0.0f);
	for (size_t i = 0; i < vertices.size() - 2; i += 2)
	{
		const Vector2f v0(vertices[i].x, vertices[0].y);
		const Vector2f& v1 = vertices[i + 1];
		const Vector2f& v2 = vertices[i + 2];

		Vector2f c = (v0 + v2) / 2.0f;

		float half_width = v2.x - c.x;
		float half_height = v1.y - c.y;

		c += other_body->position_;

		Vector2f delta_position = dynamic_body->position_ - c;

		Vector2f v = delta_position;
		v.x = clamp<float>(delta_position.x, -half_width, half_width);
		v.y = clamp<float>(delta_position.y, -half_height, half_height);

		Vector2f n = delta_position - v;

		float distance = n.compute_length() - dynamic_body_shape->radius_;
		if (distance >= 0.0f)
		{
			if (n.x < 0.0f)
			{
				break;
			}

			continue;
		}

		p_c -= n.normalize() * distance;

		has_collided = true;
	}

	if (has_collided)
	{
		float distance = p_c.compute_length();
		Vector2f n = p_c.normalized();

		if (other_body->type_ == Body::Type::STATIC)
		{
			position_correction += p_c;

			Vector2f delta_velocity = dynamic_body->velocity_ - other_body->velocity_;
			velocity_correction -= n * n.dot(delta_velocity) * (1.0f + dynamic_body->bouncing_);

			Vector2f p = n.ortho();
			velocity_correction -= p * p.dot(delta_velocity) * dynamic_body->friction_ * other_body->friction_ * 0.03f;
		}

		if (dynamic_body->collision_callback_ != nullptr)
		{
			Body::Collision collision;
			collision.collider_body = other_body;
			collision.distance = distance;
			collision.normal = n;

			dynamic_body->collision_callback_(collision);
		}
	}
}
