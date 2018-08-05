
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "avl.h"
#include "test/test.h"

/********* Private functions implementation *********/

static int _max(int x, int y) {
    if(x >= y) {
        return x;
    }
    else {
        return y;
    }
}

static avl_tree_node *_right_rotation(avl_tree_node *current_root, avl_tree_node **head) {
    avl_tree_node *left_child = current_root->left;
    avl_tree_node *previous_node = current_root->previous;
    current_root->left = left_child->right;
    left_child->right = current_root;
    if(previous_node != NULL) {
        if(current_root ==  previous_node->left) {
            previous_node->left = left_child;
        }
        else {
            previous_node->right = left_child;
        }
        left_child->previous = current_root->previous;
    }
    else {
        *head = left_child;
        left_child->previous = NULL;
    }
    current_root->previous = left_child;
    current_root->left_height = left_child->right_height;
    left_child->right_height = _max(current_root->left_height, current_root->right_height) + 1;
    return left_child;
}

static avl_tree_node *_left_rotation(avl_tree_node *current_root, avl_tree_node **head) {
    avl_tree_node *right_child = current_root->right;
    avl_tree_node *previous_node = current_root->previous;
    current_root->right = right_child->left;
    right_child->left = current_root;
    if(previous_node != NULL) {
        if(current_root ==  previous_node->left) {
            previous_node->left = right_child;
        }
        else {
            previous_node->right = right_child;
        }
        right_child->previous = current_root->previous;
    }
    else {
        *head = right_child;
        right_child->previous = NULL;
    }
    current_root->previous = right_child;
    current_root->right_height = right_child->left_height;
    right_child->left_height = _max(current_root->left_height, current_root->right_height) + 1;
    return right_child;
}

static void _search_and_insert(avl_tree_node *current_root, avl_tree_node *tmp, avl_tree_node **head) {
    avl_tree_node *pivot = current_root;
    if (pivot->value >= tmp->value) {
        if(pivot->left == NULL) {
            tmp->previous = pivot;
            pivot->left = tmp;
            pivot->left_height++;
        }
        else {
            _search_and_insert(pivot->left, tmp, head);
            pivot->left_height = _max(pivot->left->left_height, pivot->left->right_height) + 1;
        }
    }
    else {
        if(pivot->right == NULL) {
            tmp->previous = pivot;
            pivot->right = tmp;
            pivot->right_height++;
        }
        else {
            _search_and_insert(pivot->right, tmp, head);
            pivot->right_height = _max(pivot->right->left_height, pivot->right->right_height) + 1;
        }
    }

    if(pivot->left_height - pivot->right_height >= 2) {
        if(pivot->left->left_height - pivot->left->right_height <= -1) {
            pivot->left = _left_rotation(pivot->left, head);
        }
        pivot = _right_rotation(pivot, head);
    }

    if(pivot->left_height - pivot->right_height <= -2) {
        if(pivot->right->left_height - pivot->right->right_height >= 1) {
            pivot->right = _right_rotation(pivot->right, head);
        }
        pivot = _left_rotation(pivot, head);
    }
}

/********* Public functions implementation *********/
void insert_avl_node(avl_tree_node *node, avl_tree_node **avl_head) {
    if(*avl_head == NULL) {
        *avl_head = node;
        (*avl_head)->previous = NULL;
    }
    else {
        _search_and_insert(*avl_head, node, avl_head);
    }
}

void insert_value_in_avl(int value, avl_tree_node **avl_head) {
    avl_tree_node *tmp = (avl_tree_node *) calloc(1, sizeof(avl_tree_node));
    tmp->value = value;
    insert_avl_node(tmp, avl_head);
}

void delete_avl_node(avl_tree_node **avl_node) {
    avl_tree_node *pivot = *avl_node;

    if (pivot) {
        if (pivot->left) {
            delete_avl_node(&(pivot->left));
        }
        if (pivot->right) {
            delete_avl_node(&(pivot->right));
        }
        free(pivot);
        *avl_node = NULL;
    }
}

avl_tree_node *search_from_avl(int value, avl_tree_node *avl) {
    avl_tree_node *pivot = avl;
    if(pivot->value == value){
        return pivot;
    }
    else if (pivot->value > value) {
        if(pivot->left == NULL) {
            return NULL;
        }
        else {
            return search_from_avl(value, pivot->left);
        }
    }
    else {
        if(pivot->right == NULL) {
            return NULL;
        }
        else {
            return search_from_avl(value, pivot->right);
        }
    }
}

void print_avl(avl_tree_node *current_root) {
    avl_tree_node *pivot = current_root;


//    /*pre-order printing to get sorted in increasing order*/
//    if (pivot->left != NULL) {
//        print_avl(pivot->left);
//    }
//    printf("%d\t    %d\t    %d\n", pivot->value, pivot->left_height, pivot->right_height);
//    if (pivot->right != NULL) {
//        print_avl(pivot->right);
//    }


    /*in-order printing*/
    printf("%d\t    %d\t    %d\n", pivot->value, pivot->left_height, pivot->right_height);
    if (pivot->left != NULL) {
        print_avl(pivot->left);
    }
    if (pivot->right != NULL) {
        print_avl(pivot->right);
    }


//    /*post-order printing to get sorted in decreasing order*/
//    if (pivot->right != NULL) {
//        print_avl(pivot->right);
//    }
//    printf("%d\t    %d\t    %d\n", pivot->value, pivot->left_height, pivot->right_height);
//    if (pivot->left != NULL) {
//        print_avl(pivot->left);
//    }

}

/********* Test functions *********/
int test_create_avl(avl_tree_node **avl_head) {
    int count = 0;
    int val;
    while (true) {
        avl_tree_node *tmp = (avl_tree_node *) calloc(1, sizeof(avl_tree_node));
        printf("Enter non-zero number ('0' to exit):- ");
        scanf("%d", &val);
        if(!val) {
            break;
        }
        count++;
        tmp->value = val;
        insert_avl_node(tmp, avl_head);
    }

    printf("total number of nodes = %d\n", count);
    printf("\nvalue\tleft\tright\n");
    printf("----------------------\n");
    print_avl(*avl_head);

    return 0;
}

int test_search_avl(avl_tree_node **avl_head) {
    int value = 1;

    while(value){
        printf("Enter non-zero number ('0' to exit):- ");
        scanf("%d", &value);
        avl_tree_node *t= search_from_avl(value, *avl_head);
        if(t != NULL)
            printf("found: %d\n", t->value);
        else
            printf("not found\n");
    }

    return 0;
}

int test_avl() {
    avl_tree_node *head;

    printf("Creating AVL Tree ... \n\n");
    test_create_avl(&head);

    printf("Searching AVL Tree ... \n\n");
    test_search_avl(&head);

    return 0;
}