#include <exit.h>
#include <stdio.h>
#include <ds/trees.h>

void printf_visit(struct binary_tree_node *node);

void _start() {
	struct binary_tree *tree = create_binary_tree(1);
	struct binary_tree_node *node = create_binary_tree_node("Root tree", 0, 0);
	tree->root = node;

	tree->root->left = create_binary_tree_node("Left of root", 0 , 0);
	tree->root->right = create_binary_tree_node("Right of root", 0 , 0);
	tree->root->left->left = create_binary_tree_node("Left of left root", 0, 0);

	tree->ops->inorder_traverse(tree->root, printf_visit);
	tree->ops->postorder_traverse(tree->root, printf_visit);
	tree->ops->preorder_traverse(tree->root, printf_visit);
	exit_app();
}


void printf_visit(struct binary_tree_node *node) {
	printf("Node value : %s\n", (char *)node->data);
}
