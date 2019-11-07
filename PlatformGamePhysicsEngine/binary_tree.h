#pragma once

#include <vector>
#include "body.h"
#include "utility.h"

class BinaryTree
{
public:
	struct Ball;

	BinaryTree(float partition_width);
	~BinaryTree();

	void add_ball(Ball* ball);
	static void remove_ball(Ball* ball);
	void update(Ball* ball);

private:
	struct Node;
	struct Branch;
	struct Leaf;

	const float partition_width_;

	Branch* root_;

	Leaf* find_leaf(float x);
	void expand_on_right();
	void expand_on_left();
	Node* grow_on_right(Branch* branch, Node* right_brother);
	Node* grow_on_left(Branch* branch, Node* left_brother);
};

struct BinaryTree::Node
{
	Node* father;

	float min_x;
	float max_x;

	bool is_leaf;

	Node(Node* father, float min_x, float max_x, bool is_leaf);
	virtual ~Node() {}
};

struct BinaryTree::Branch :public Node
{
	Node* left_child;
	Node* right_child;

	Branch(Node* father, float min_x, float max_x);
	~Branch();
};

struct BinaryTree::Leaf :public Node
{
	Leaf* left_brother;
	Leaf* right_brother;

	std::vector<Ball*> balls;

	Leaf(Node* father, float min_x, float max_x);
};

struct BinaryTree::Ball
{
	friend class BinaryTree;

	Body* body;
	std::vector<Body*> bodies;

	Ball(Body* body);

private:
	Leaf* left_leaf;
	Leaf* right_leaf;
};
