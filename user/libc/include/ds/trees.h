#include <stdint.h>

struct binary_tree_node;

struct binary_tree_node {
	void *data;
	struct binary_tree_node *left;
	struct binary_tree_node *right;
};

typedef void (*visit_node)(struct binary_tree_node *node);
typedef void (*traverse)(struct binary_tree_node *node, visit_node visit);

struct binary_tree_ops {
	traverse preorder_traverse;
	traverse postorder_traverse;
	traverse inorder_traverse;
};

struct binary_tree {
	struct binary_tree_node *root;
	struct binary_tree_ops *ops;
};

struct binary_tree_node *create_binary_tree_node(void *data, struct binary_tree_node *left, struct binary_tree_node *right);
struct binary_tree *create_binary_tree(int recusive);
