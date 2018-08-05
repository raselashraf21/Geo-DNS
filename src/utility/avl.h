
#ifndef GEODNS_AVL_H
#define GEODNS_AVL_H

typedef struct _avl_tree_node {
    struct _avl_tree_node *left;
    struct _avl_tree_node *right;
    struct _avl_tree_node *previous;
    int value, left_height, right_height;
} avl_tree_node;

void insert_avl_node(avl_tree_node *node, avl_tree_node **avl);
void insert_value_in_avl(int value, avl_tree_node **avl);
void delete_avl_node(avl_tree_node **avl_node);
avl_tree_node *search_from_avl(int value, avl_tree_node *avl);

void print_avl(avl_tree_node *avl);

#endif // GEODNS_AVL_H
