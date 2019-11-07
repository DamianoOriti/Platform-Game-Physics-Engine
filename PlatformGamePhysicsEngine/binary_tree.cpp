#include "binary_tree.h"

BinaryTree::BinaryTree(float partition_width) :
	partition_width_(partition_width),
	root_(nullptr)
{
	float total_width = partition_width * 2.0f;

	root_ = new Branch(nullptr, 0.0f, total_width);

	Leaf* left_leaf = new Leaf(root_, 0.0f, partition_width);
	Leaf* right_leaf = new Leaf(root_, partition_width, total_width);

	left_leaf->left_brother = nullptr;
	left_leaf->right_brother = right_leaf;
	right_leaf->left_brother = left_leaf;
	right_leaf->right_brother = nullptr;

	root_->left_child = left_leaf;
	root_->right_child = right_leaf;
}

BinaryTree::~BinaryTree()
{
	delete root_;
}

void BinaryTree::add_ball(Ball* ball)
{
	// expand the binary tree if necessary
	while (root_->min_x > ball->body->min_x_)
	{
		expand_on_left();
	}
	while (root_->max_x < ball->body->max_x_)
	{
		expand_on_right();
	}

	// find the leaf which can contain the extreme left side of the body
	ball->left_leaf = find_leaf(ball->body->min_x_);

	Leaf* current_leaf = ball->left_leaf;
	do
	{
		for each (auto leaf_ball in current_leaf->balls)
		{
			leaf_ball->bodies.push_back(ball->body);
			push_unique<Body>(leaf_ball->body, ball->bodies);
		}

		current_leaf->balls.push_back(ball);

		if (current_leaf->right_brother == nullptr)
		{
			ball->right_leaf = current_leaf;
			return;
		}

		current_leaf = current_leaf->right_brother;

	} while (ball->body->max_x_ > current_leaf->min_x);

	ball->right_leaf = current_leaf->left_brother;
}

void BinaryTree::remove_ball(Ball* ball)
{
	Leaf* current_leaf = ball->left_leaf;
	do
	{
		pop<Ball>(ball, current_leaf->balls);

		for each (auto leaf_ball in current_leaf->balls)
		{
			pop<Body>(ball->body, leaf_ball->bodies);
			pop<Body>(leaf_ball->body, ball->bodies);
		}

		current_leaf = current_leaf->right_brother;

	} while (current_leaf != nullptr
		&& current_leaf->min_x < ball->body->max_x_);

	ball->bodies.clear();
	ball->left_leaf = nullptr;
	ball->right_leaf = nullptr;
}

void BinaryTree::update(Ball* ball)
{
	Body* body = ball->body;
	if (body->min_x_ >= ball->left_leaf->min_x
		&& body->min_x_ <= ball->left_leaf->max_x
		&& body->max_x_ >= ball->right_leaf->min_x
		&& body->max_x_ <= ball->right_leaf->max_x)
	{
		return;
	}

	// if body has moved to the left and its extreme left side has moved to the
	// left of the current left leaf, make the left brother of the current left
	// leaf the new left leaf
	if (body->min_x_ < ball->left_leaf->min_x)
	{
		// if the old left leaf has no left brother, expand the tree on the left
		if (ball->left_leaf->left_brother == nullptr)
		{
			expand_on_left();
		}

		ball->left_leaf = ball->left_leaf->left_brother;

		for each (auto leaf_ball in ball->left_leaf->balls)
		{
			push_unique<Body>(ball->body, leaf_ball->bodies);
			push_unique<Body>(leaf_ball->body, ball->bodies);
		}

		ball->left_leaf->balls.push_back(ball);
	}

	// if the body extreme right side has moved to the left of the current right leaf,
	// make the left brother of the current right leaf the new right leaf
	if (body->max_x_ < ball->right_leaf->min_x)
	{
		pop<Ball>(ball, ball->right_leaf->balls);

		for each (auto leaf_ball in ball->right_leaf->balls)
		{
			if (ball->right_leaf == leaf_ball->left_leaf)
			{
				pop<Body>(ball->body, leaf_ball->bodies);
				pop<Body>(leaf_ball->body, ball->bodies);
			}
		}

		ball->right_leaf = ball->right_leaf->left_brother;
	}

	// ...or body has moved to the right and...
	if (body->max_x_ > ball->right_leaf->max_x)
	{
		// if the old right leaf has no right brother, expand the tree on the right
		if (ball->right_leaf->right_brother == nullptr)
		{
			expand_on_right();
		}

		ball->right_leaf = ball->right_leaf->right_brother;

		for each (auto leaf_ball in ball->right_leaf->balls)
		{
			push_unique<Body>(ball->body, leaf_ball->bodies);
			push_unique<Body>(leaf_ball->body, ball->bodies);
		}

		ball->right_leaf->balls.push_back(ball);
	}

	// if the body extreme left side has moved to the right of the current left leaf,
	// make the right brother of the current left leaf the new left leaf
	if (body->min_x_ > ball->left_leaf->max_x)
	{
		pop<Ball>(ball, ball->left_leaf->balls);

		for each (auto leaf_ball in ball->left_leaf->balls)
		{
			if (ball->left_leaf == leaf_ball->right_leaf)
			{
				pop<Body>(ball->body, leaf_ball->bodies);
				pop<Body>(leaf_ball->body, ball->bodies);
			}
		}

		ball->left_leaf = ball->left_leaf->right_brother;
	}
}

BinaryTree::Leaf* BinaryTree::find_leaf(float x)
{
	Branch* current_node = root_;
	do
	{
		float mean_x = (current_node->min_x + current_node->max_x) / 2.0f;
		if (mean_x < x)
		{
			current_node = static_cast<Branch*>(current_node->right_child);
		}
		else
		{
			current_node = static_cast<Branch*>(current_node->left_child);
		}

	} while (!current_node->is_leaf);

	return reinterpret_cast<Leaf*>(current_node);
}

void BinaryTree::expand_on_right()
{
	Branch* old_root = root_;

	float min_x = old_root->min_x;
	float max_x = old_root->min_x + (old_root->max_x - old_root->min_x) * 2.0f;
	root_ = new Branch(nullptr, min_x, max_x);

	root_->left_child = old_root;
	root_->left_child->father = root_;
	root_->right_child = new Branch(old_root, old_root->max_x, root_->max_x);

	// find the first leaf on the right starting from the old root
	Branch* current_node = old_root;
	do
	{
		current_node = static_cast<Branch*>(current_node->right_child);

	} while (!current_node->is_leaf);

	grow_on_left(static_cast<Branch*>(root_->right_child), current_node);
}

void BinaryTree::expand_on_left()
{
	Branch* old_root = root_;

	float min_x = old_root->max_x + (old_root->min_x - old_root->max_x) * 2.0f;
	float max_x = old_root->max_x;
	root_ = new Branch(nullptr, min_x, max_x);

	root_->right_child = old_root;
	root_->right_child->father = root_;
	root_->left_child = new Branch(old_root, root_->min_x, old_root->min_x);

	// find the first leaf on the left starting from the old root
	Branch* current_node = old_root;
	do
	{
		current_node = static_cast<Branch*>(current_node->left_child);

	} while (!current_node->is_leaf);

	grow_on_right(static_cast<Branch*>(root_->left_child), current_node);
}

BinaryTree::Node* BinaryTree::grow_on_right(Branch* branch, Node* right_brother)
{
	// this is a branch with leaves
	if (branch->max_x - branch->min_x == partition_width_ * 2.0f)
	{
		float mean_x = (branch->min_x + branch->max_x) / 2.0f;
		Leaf* left_leaf = new Leaf(branch, branch->min_x, mean_x);

		Leaf* right_leaf = new Leaf(branch, mean_x, branch->max_x);

		right_leaf->right_brother = static_cast<Leaf*>(right_brother);
		right_leaf->right_brother->left_brother = right_leaf;
		right_leaf->left_brother = left_leaf;
		left_leaf->right_brother = right_leaf;
		left_leaf->left_brother = nullptr;

		branch->left_child = left_leaf;
		branch->right_child = right_leaf;

		return branch->left_child;
	}

	float mean_x = (branch->min_x + branch->max_x) / 2.0f;

	branch->right_child = new Branch(branch, mean_x, branch->max_x);
	Node* r_b = grow_on_right(static_cast<Branch*>(branch->right_child), right_brother);

	branch->left_child = new Branch(branch, branch->min_x, mean_x);
	r_b = grow_on_right(static_cast<Branch*>(branch->left_child), r_b);

	return r_b;
}

BinaryTree::Node* BinaryTree::grow_on_left(Branch* branch, Node* left_brother)
{
	// this is a branch with leaves
	if (branch->max_x - branch->min_x == partition_width_ * 2.0f)
	{
		float mean_x = (branch->min_x + branch->max_x) / 2.0f;
		Leaf* left_leaf = new Leaf(branch, branch->min_x, mean_x);

		Leaf* right_leaf = new Leaf(branch, mean_x, branch->max_x);

		left_leaf->left_brother = static_cast<Leaf*>(left_brother);
		left_leaf->left_brother->right_brother = left_leaf;
		left_leaf->right_brother = right_leaf;
		right_leaf->left_brother = left_leaf;
		right_leaf->right_brother = nullptr;

		branch->left_child = left_leaf;
		branch->right_child = right_leaf;

		return branch->right_child;
	}

	float mean_x = (branch->min_x + branch->max_x) / 2.0f;

	branch->left_child = new Branch(branch, branch->min_x, mean_x);
	Node* l_b = grow_on_left(static_cast<Branch*>(branch->left_child), left_brother);

	branch->right_child = new Branch(branch, mean_x, branch->max_x);
	l_b = grow_on_left(static_cast<Branch*>(branch->right_child), l_b);

	return l_b;
}

BinaryTree::Node::Node(Node* father, float min_x, float max_x, bool is_leaf) :
	min_x(min_x),
	max_x(max_x),
	father(father),
	is_leaf(is_leaf)
{
}

BinaryTree::Branch::Branch(Node* father, float min_x, float max_x) :
	Node(father, min_x, max_x, false),
	left_child(nullptr),
	right_child(nullptr)
{
}

BinaryTree::Branch::~Branch()
{
	if (left_child != nullptr)
	{
		delete left_child;
		left_child = nullptr;
	}

	if (right_child != nullptr)
	{
		delete right_child;
		right_child = nullptr;
	}
}

BinaryTree::Leaf::Leaf(Node* father, float min_x, float max_x) :
	Node(father, min_x, max_x, true),
	left_brother(nullptr),
	right_brother(nullptr)
{
}

BinaryTree::Ball::Ball(Body* body) :
	body(body)
{
}
