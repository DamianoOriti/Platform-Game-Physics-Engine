#include <iostream>
#include <algorithm>
#include "physics_engine.h"
#include <GL\glut.h>

//#define RUN_FULLSCREEN

enum Entity_Id
{
	TERRAIN,
	COIN,
	STAR,
	GOOMBA
};

std::vector<Body*> bodies_to_be_deleted;

class Mario
{
public:
	Body* body_;
	bool is_airborne_;
	Vector2f impulse_;
	bool is_super_ = false;
	bool should_become_super_ = false;
	bool should_become_small_ = false;
	bool can_jump_ = true;

	void update()
	{
		if (is_super_ && should_become_small_)
		{
			Capsule_Shape* shape = reinterpret_cast<Capsule_Shape*>(body_->get_shape());
			shape->set_distance(0.0f);

			should_become_small_ = false;
			is_super_ = false;
		}

		if (!is_super_ && should_become_super_)
		{
			Capsule_Shape* shape = reinterpret_cast<Capsule_Shape*>(body_->get_shape());
			shape->set_distance(0.5f);

			should_become_super_ = false;
			is_super_ = true;
		}

		Vector2f imp = impulse_;
		if (is_airborne_)
		{
			imp.x *= 0.125f;
			imp.y = 0.0f;
		}
		else
		{
			if (can_jump_)
			{
				can_jump_ = false;
			}
			else
			{
				//imp.y = 0.0f;
			}
		}
		body_->apply_impulse(imp);

		is_airborne_ = true;
	}

	void collision_callback(Body::Collision& collision)
	{
		int entity_id = reinterpret_cast<int>(collision.collider_body->get_entiry());
		switch (entity_id)
		{
		case Entity_Id::TERRAIN:
			if (collision.normal.y == 1.0f)
			{
				is_airborne_ = false;
			}
			break;
		case Entity_Id::COIN:
			std::cout << "mario: got coin!" << std::endl;
			bodies_to_be_deleted.push_back(collision.collider_body);
			break;
		case Entity_Id::STAR:
			std::cout << "mario: got mushroom!" << std::endl;
			should_become_super_ = !is_super_;
			bodies_to_be_deleted.push_back(collision.collider_body);
			break;
		case Entity_Id::GOOMBA:
		{
			std::cout << "mario: collising with goomba!" << std::endl;

			float m = collision.normal.y / collision.normal.x;
			if (m > 1.0f || m < -1.0f)
			{
				Vector2f impulse(collision.normal * 10.0f);
				body_->apply_impulse(impulse);

				bodies_to_be_deleted.push_back(collision.collider_body);
			}
			else if (is_super_)
			{
				should_become_small_ = true;
			}
			else
			{
				std::cout << "mario: should die!" << std::endl;
			}

			break;
		}
		default:
			break;
		}
	}
};

class Goomba
{
public:
	Body* body_;
	float velocity_x_ = -2.0f;

	void update()
	{
		body_->velocity_.x = velocity_x_;
	}

	void collision_callback(Body::Collision& collision)
	{
		int entity_id = reinterpret_cast<int>(collision.collider_body->get_entiry());
		switch (entity_id)
		{
		case Entity_Id::TERRAIN:
			//std::cout << "goomba: colliding with terrain!" << std::endl;

			if (collision.normal.x != 0.0f)
			{
				velocity_x_ = (collision.normal.x / fabs(collision.normal.x)) * 2.0f;
			}
			break;
		case Entity_Id::GOOMBA:
			std::cout << "goomba: colliding with goomba!" << std::endl;

			if (collision.normal.x != 0.0f)
			{
				velocity_x_ = (collision.normal.x / fabs(collision.normal.x)) * 2.0f;
			}
			break;
		default:
			break;
		}
	}
};


const float deltaTime = 1.0f / 144.0f;
const std::vector<Vector2f> vertices = {
	{0.0f, 0.0f},
	{0.0f, 5.0f},
	{10.0f, 5.0f},
	{10.0f, 8.0f},
	{12.0f, 8.0f},
	{12.0f, 5.0f},
	{18.0f, 5.0f},
	{18.0f, 9.0f},
	{20.0f, 9.0f},
	{20.0f, 5.0f},
	{22.0f, 5.0f},
	{22.0f, 0.0f}
};


Physics_Engine* physics_engine(nullptr);
Mario* mario(nullptr);
std::vector<Goomba*> goombas;
std::vector<Body*> bodies;
Vector2f camera_position(0.0f, 0.0f);


void release_data()
{
	if (mario != nullptr)
	{
		delete mario;
	}

	for (size_t i = 0; i < goombas.size(); i++)
	{
		delete goombas[i];
	}

	for (size_t i = 0; i < bodies.size(); i++)
	{
		delete bodies[i];
	}

	if (physics_engine != nullptr)
	{
		delete physics_engine;
	}
}

void load_data()
{
	Shape* shape(nullptr);
	Body* body(nullptr);

	try
	{
		Vector2f gravity(0.0f, -9.81f);
		physics_engine = new Physics_Engine(gravity);

		Body::Type type;
		Vector2f position;
		void* entity(nullptr);

		// terrain
		{
			entity = reinterpret_cast<void*>(Entity_Id::TERRAIN);
			type = Body::Type::STATIC;
			position = Vector2f(0.0f, -4.0f);
			shape = new Chain_Shape(vertices);
			body = new Body(type, position, shape, nullptr, entity);
			body->bouncing_ = 0.0f;
			body->friction_ = 1.0f;
			physics_engine->add_body(body);
			bodies.push_back(body);
			shape = nullptr;
			body = nullptr;
		}

		// coins
		entity = reinterpret_cast<void*>(Entity_Id::COIN);
		for (size_t i = 0; i < 5; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				type = Body::Type::SENSOR;
				position = Vector2f(0.5f + i * 2.0f, 4.0f + j * 2.0f);
				shape = new Circle_Shape(0.5f);
				body = new Body(type, position, shape, nullptr, entity);
				body->bouncing_ = 0.0f;
				body->friction_ = 1.0f;
				physics_engine->add_body(body);
				bodies.push_back(body);
				shape = nullptr;
				body = nullptr;
			}
		}

		// goombas
		{
			Goomba* goomba = new Goomba;
			goombas.push_back(goomba);
			entity = reinterpret_cast<void*>(Entity_Id::GOOMBA);
			type = Body::Type::DYNAMIC;
			position = Vector2f(9.0f, 2.0f);
			shape = new Circle_Shape(0.5f);
			body = new Body(
				type,
				position,
				shape,
				std::bind(
					&Goomba::collision_callback,
					goomba,
					std::placeholders::_1
				),
				entity
			);
			body->bouncing_ = 0.0f;
			body->friction_ = 0.0f;
			body->apply_impulse(Vector2f(-2.0f, 0.0f));
			physics_engine->add_body(body);
			bodies.push_back(body);
			goomba->body_ = body;
			shape = nullptr;
			body = nullptr;

			goomba = new Goomba;
			goombas.push_back(goomba);
			entity = reinterpret_cast<void*>(Entity_Id::GOOMBA);
			type = Body::Type::DYNAMIC;
			position = Vector2f(14.0f, 2.0f);
			shape = new Circle_Shape(0.5f);
			body = new Body(
				type,
				position,
				shape,
				std::bind(
					&Goomba::collision_callback,
					goomba,
					std::placeholders::_1
				),
				entity
			);
			body->bouncing_ = 0.0f;
			body->friction_ = 0.0f;
			body->apply_impulse(Vector2f(-2.0f, 0.0f));
			physics_engine->add_body(body);
			bodies.push_back(body);
			goomba->body_ = body;
			shape = nullptr;
			body = nullptr;

			goomba = new Goomba;
			goombas.push_back(goomba);
			entity = reinterpret_cast<void*>(Entity_Id::GOOMBA);
			type = Body::Type::DYNAMIC;
			position = Vector2f(16.0f, 2.0f);
			shape = new Circle_Shape(0.5f);
			body = new Body(
				type,
				position,
				shape,
				std::bind(
					&Goomba::collision_callback,
					goomba,
					std::placeholders::_1
				),
				entity
			);
			body->bouncing_ = 0.0f;
			body->friction_ = 0.0f;
			body->apply_impulse(Vector2f(-2.0f, 0.0f));
			physics_engine->add_body(body);
			bodies.push_back(body);
			goomba->body_ = body;
			shape = nullptr;
			body = nullptr;
		}

		// mario
		entity = nullptr;
		mario = new Mario;
		type = Body::Type::DYNAMIC;
		position = Vector2f(1.0f, 8.0f);
		shape = new Capsule_Shape(0.5f, 0.0f);
		body = new Body(
			type,
			position,
			shape,
			std::bind(
				&Mario::collision_callback,
				mario,
				std::placeholders::_1
			),
			entity
		);
		body->bouncing_ = 0.0f;
		body->friction_ = 1.0f;
		physics_engine->add_body(body);
		bodies.push_back(body);
		mario->body_ = body;
		shape = nullptr;
		body = nullptr;

		// mushroom
		entity = reinterpret_cast<void*>(Entity_Id::STAR);
		type = Body::Type::DYNAMIC;
		position = Vector2f(12.0f, 4.0f);
		shape = new Circle_Shape(0.5f);
		body = new Body(type, position, shape, nullptr, entity);
		body->bouncing_ = 1.0f;
		body->friction_ = 0.0f;
		body->velocity_.x = 3.0f;
		physics_engine->add_body(body);
		bodies.push_back(body);
		shape = nullptr;
		body = nullptr;
	}
	catch (...)
	{
		if (body != nullptr)
		{
			delete body;
			body = nullptr;
		}

		if (shape != nullptr)
		{
			delete shape;
			shape = nullptr;
		}

		throw;
	}
}

void idle()
{
	physics_engine->update(deltaTime);

	for each (auto body in bodies_to_be_deleted)
	{
		physics_engine->remove_body(body);

		for (auto it = bodies.begin(); it != bodies.end(); it++)
		{
			if ((*it) == body)
			{
				bodies.erase(it);
				break;
			}
		}
	}
	bodies_to_be_deleted.clear();

	mario->update();

	for each (auto goomba in goombas)
	{
		goomba->update();
	}

	camera_position.lerp(mario->body_->get_position(), 0.05f);

	glutPostRedisplay();
}

void draw_shape(const Shape* shape)
{
	switch (shape->get_type())
	{
	case Shape::Type::BOX:
	{
		const Box_Shape* box_shape = static_cast<const Box_Shape*>(shape);

		float half_width = box_shape->get_half_width();
		float half_height = box_shape->get_half_height();

		glBegin(GL_LINE_LOOP);
		glVertex2f(-half_width, -half_height);
		glVertex2f(-half_width, half_height);
		glVertex2f(half_width, half_height);
		glVertex2f(half_width, -half_height);
		glEnd();
		break;
	}
	case Shape::Type::CIRCLE:
	{
		const Circle_Shape* circle_shape = static_cast<const Circle_Shape*>(shape);
		float radius = circle_shape->get_radius();

		glBegin(GL_LINE_LOOP);
		for (size_t i = 0; i < 64; i++)
		{
			glVertex2f(radius * cosf(i * 0.098125f), radius * sinf(i * 0.098125f));
		}
		glEnd();
		break;
	}
	case Shape::Type::CAPSULE:
	{
		const Capsule_Shape* capsule_shape = static_cast<const Capsule_Shape*>(shape);

		float radius = capsule_shape->get_radius();
		float distance = capsule_shape->get_distance();

		glBegin(GL_LINE_LOOP);
		for (size_t i = 0; i < 32; i++)
		{
			glVertex2f(radius * cosf(i * -0.098125f), radius * sinf(i * -0.098125f));
		}

		for (size_t i = 32; i < 64; i++)
		{
			glVertex2f(radius * cosf(i * -0.098125f), radius * sinf(i * -0.098125f) + distance);
		}
		glEnd();
		break;
	}
	case Shape::Type::CHAIN:
	{
		const Chain_Shape* chain_shape = static_cast<const Chain_Shape*>(shape);

		const auto& vertices = chain_shape->get_vertices();

		glBegin(GL_LINE_STRIP);
		//glBegin(GL_LINE_LOOP);
		for (size_t i = 0; i < vertices.size(); i++)
		{
			glVertex2f(vertices[i].x, vertices[i].y);
		}
		glEnd();
		break;
	}
	default:
		break;
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(
		camera_position.x, camera_position.y, 1.0,
		camera_position.x, camera_position.y, 0.0,
		0.0, 1.0, 0.0
	);

	// draw bodies
	for each (auto body in bodies)
	{
		Body::Type type = body->get_type();
		if (type == Body::Type::DYNAMIC)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else if (type == Body::Type::SENSOR)
		{
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		else
		{
			glColor3f(0.0f, 1.0f, 0.0f);
		}

		glPushMatrix();

		const Vector2f& position = body->get_position();
		glTranslatef(position.x, position.y, 0.0f);

		draw_shape(body->get_shape());

		glPopMatrix();
	}

	glutSwapBuffers();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLdouble ratio = static_cast<GLdouble>(height) / static_cast<GLdouble>(width);
	glOrtho(-16.0, 16.0, -16.0 * ratio, 16.0 * ratio, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		release_data();
		exit(0);
	case 'd':
		mario->impulse_.x = 0.1f;
		break;
	case 'a':
		mario->impulse_.x = -0.1f;
		break;
	case 'w':
		mario->impulse_.y = 8.0f;
		break;
	case 's':
		mario->impulse_.y = -0.5f;
		break;
	default:
		break;
	}
}

void keyboard_up(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		release_data();
		exit(0);
	case 'd':
		if (mario->impulse_.x > 0.0f)
		{
			mario->impulse_.x = 0.0f;
		}
		break;
	case 'a':
		if (mario->impulse_.x < 0.0f)
		{
			mario->impulse_.x = 0.0f;
		}
		break;
	case 'w':
		if (mario->impulse_.y > 0.0f)
		{
			mario->impulse_.y = 0.0f;
		}
		mario->can_jump_ = true;
		break;
	case 's':
		if (mario->impulse_.y < 0.0f)
		{
			mario->impulse_.y = 0.0f;
		}
		break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	try
	{
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(200, 200);
		glutInitWindowSize(800, 600);
		glutCreateWindow("Physics Engine");
		glutSetCursor(GLUT_CURSOR_NONE);
		glutIdleFunc(idle);
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutKeyboardUpFunc(keyboard_up);

		glClearColor(0.25f, 0.25f, 0.25f, 0.0f);

		load_data();

#ifdef RUN_FULLSCREEN
		glutFullScreen();
#endif // RUN_FULLSCREEN

		glutMainLoop();
	}
	catch (...)
	{
		release_data();
		return -1;
	}
}
