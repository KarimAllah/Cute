#include <malloc.h>
#include <ds/trees.h>
#include <ds/lists.h>

static void binary_preorder_tree_traverse_iterative(struct binary_tree_node *node, visit_node visit)
{
	struct stack *memory = create_stack();
	struct binary_tree_node *current = node;
reiterate:
	if(!node)
	{
		current = stack_pop(memory);
		if (!current)
			return;
	}

	// visit the current
	visit(current);

	// remember the other path
	stack_push(memory, node->right);

	// traverse the left side
	current = node->left;
	goto reiterate;
}

static void binary_inorder_tree_traverse_iterative(struct binary_tree_node *node, visit_node visit)
{
	struct stack *memory = create_stack();
	struct binary_tree_node *current = node;

go_left:
	if(!current)
		return;

	// remember the other path
	stack_push(memory, current->left);

	if(!current->left) {
		visit(current);
		current = stack_pop(memory);
		visit(current);
		current = current->right;
	}
	goto go_left;
}

static void binary_postorder_tree_traverse_iterative(struct binary_tree_node *node, visit_node visit)
{
	// TODO:
}

static void binary_preorder_tree_traverse_recursive(struct binary_tree_node *node, visit_node visit)
{
	if (!node)
		return;

	visit(node);
	binary_preorder_tree_traverse_recursive(node->left, visit);
	binary_preorder_tree_traverse_recursive(node->right, visit);
}

static void binary_inorder_tree_traverse_recursive(struct binary_tree_node *node, visit_node visit)
{
	if (!node)
		return;

	binary_inorder_tree_traverse_recursive(node->left, visit);
	visit(node);
	binary_inorder_tree_traverse_recursive(node->right, visit);
}

static void binary_postorder_tree_traverse_recursive(struct binary_tree_node *node, visit_node visit)
{
	if (!node)
		return;

	binary_postorder_tree_traverse_recursive(node->left, visit);
	binary_postorder_tree_traverse_recursive(node->right, visit);
	visit(node);
}

struct binary_tree_node *create_binary_tree_node(void *data, struct binary_tree_node *left, struct binary_tree_node *right)
{
	struct binary_tree_node *node = (struct binary_tree_node *)malloc(sizeof(struct binary_tree_node));
	node->data = data;
	node->left = left;
	node->right = right;
	return node;
}

struct binary_tree *create_binary_tree(int recusive)
{
	struct binary_tree *tree = (struct binary_tree *)malloc(sizeof(struct binary_tree));
	struct binary_tree_ops *ops = (struct binary_tree_ops *)malloc(sizeof(struct binary_tree_ops));

	if(recusive) {
		ops->inorder_traverse = binary_inorder_tree_traverse_recursive;
		ops->preorder_traverse = binary_preorder_tree_traverse_recursive;
		ops->postorder_traverse = binary_postorder_tree_traverse_recursive;
	} else {
		ops->inorder_traverse = binary_inorder_tree_traverse_iterative;
		ops->preorder_traverse = binary_preorder_tree_traverse_iterative;
		ops->postorder_traverse = binary_postorder_tree_traverse_iterative;
	}

	tree->ops = ops;
	tree->root = create_binary_tree_node(0, 0, 0);
	return tree;
}
