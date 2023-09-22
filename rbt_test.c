#include <stdlib.h>
#include <stdio.h>
#include "rbt.h"
#include "fbaa.h"

#define MAX_NODES 16

fbaa_t* rbt_allocator;

rbt_node_t* rbt_malloc() {
    return (rbt_node_t*) fbaa_malloc(rbt_allocator);
}

void rbt_free(rbt_node_t* node) {
    fbaa_free(rbt_allocator, (void*) node);
}

int assert_rbt_ndr(rbt_node_t* node) {
    if(node == (rbt_node_t*)0) {
        return 0;
    }
    if(rbt_is_red(node->left)) {
        if(rbt_is_red(node->left->left)) {
            return 1;
        }
    }
    return assert_rbt_ndr(node->left)
        + assert_rbt_ndr(node->right);
}

int assert_rbt_nrr(
    rbt_node_t* node
) {
    if(node == (rbt_node_t*)0) {
        return 0;
    }
    if(rbt_is_red(node->right)) {
        return 1;
    }

    return assert_rbt_nrr(node->left)
        + assert_rbt_nrr(node->right);
}

int assert_rbt_count(
    rbt_node_t* node, int black_count, int* valid
) {
    if(node == (rbt_node_t*)0) {
        return 1;
    }

    int left = assert_rbt_count(node->left, black_count, valid);
    int right = assert_rbt_count(node->right, black_count, valid);
    if(right != left) {
        *valid += 1;
    }

    if(node->color == BLACK) {
        return 1 + left;
    }

    return left;
}

int rbt_inorder(rbt_node_t* node, int* vals, int index) {
    if(node == (rbt_node_t*)0) {
        return index;
    }
    index = rbt_inorder(node->left, vals, index);
    vals[index++] = node->val;
    return rbt_inorder(node->right, vals, index);
}

void verify_rbt(rbt_t* rbt, int* vals, int size) {
    if(rbt->size != size) {
        printf("\tERROR: rbt size not %d: %d\n", size, rbt->size);
    }
    {
        int valid = 0;
        assert_rbt_count(rbt->root, 0, &valid);
        if(valid != 0) {
            printf("\tERROR: rbt count invalid\n");
        }
    }
    {
        int valid = assert_rbt_nrr(rbt->root);
        if(valid != 0) {
            printf("\tERROR: rbt has right red child\n");
        }
    }
    {
        int valid = assert_rbt_ndr(rbt->root);
        if(valid != 0) {
            printf("\tERROR: rbt subsequent red left children\n");
        }
    }
    if(size > 0) {
        int inorder[MAX_NODES];
        rbt_inorder(rbt->root, inorder, 0);
        int valid = 0;
        for(int i = 0; i < size; ++i) {
            if(vals[i] != inorder[i]) {
                valid = 1;
                break;
            }
        }
        if(valid != 0) {
            printf("\tERROR: rbt invalid inorder values\n");
            printf("\t");
            for(int i = 0; i < size; ++i) {
                printf("\t%d ", vals[i]);
            }
            printf("\n\t");
            for(int i = 0; i < size; ++i) {
                printf("\t%d ", inorder[i]);
            }
            printf("\n");
        }
    }
}

void test_empty() {
    printf("TEST EMPTY TREE\n");
    fbaa_clear(rbt_allocator);
    rbt_t rbt;
    rbt_init(&rbt);
    verify_rbt(&rbt, (int*)0, 0);
}

void test_singleton() {
    printf("TEST SINGLETON TREE\n");
    fbaa_clear(rbt_allocator);
    rbt_t rbt;
    rbt_init(&rbt);
    rbt_node_t* node = rbt_malloc();
    rbt_init_node(node, 3, 3);
    rbt_insert(&rbt, node);
    int vals[1] = { 3 };
    verify_rbt(&rbt, vals, 1);
}

void test_many_insertions() {
    printf("TEST MANY INSERTIONS\n");
    fbaa_clear(rbt_allocator);
    rbt_t rbt;
    rbt_init(&rbt);

    int vals[7] = { 1, 2, 3, 5, 8, 11, 17 };
    for(int i = 0; i < 7; ++i) {
        rbt_node_t* node = rbt_malloc();
        int index = (i * 5) % 7;
        rbt_init_node(node, vals[index], vals[index]);
        rbt_insert(&rbt, node);
    }
    verify_rbt(&rbt, vals, 7);
}

void test_delete() {
    printf("TEST DELETE\n");
    fbaa_clear(rbt_allocator);
    rbt_t rbt;
    rbt_init(&rbt);

    int vals[7] = { 1, 3, 4, 7, 8, 13, 14 };
    for(int i = 0; i < 7; ++i) {
        rbt_node_t* node = rbt_malloc();
        int index = (i * 5) % 7;
        rbt_init_node(node, vals[index], vals[index]);
        rbt_insert(&rbt, node);
    }
    rbt_node_t* node = rbt_malloc();
    rbt_init_node(node, 5, 5);
    rbt_insert(&rbt, node);
    rbt_node_t* fnode = rbt_delete(&rbt, 5);
    if(fnode == (rbt_node_t*)0) {
        printf("\tERROR: delete returned null\n");
    } else if(5 != fnode->key) {
        printf(
            "\tERROR: deleted node does not match key %d: %ld\n",
            5, fnode->key
        );
    }

    verify_rbt(&rbt, vals, 7);
}

void test_delete_all() {
    printf("TEST DELETE ALL\n");
    fbaa_clear(rbt_allocator);
    rbt_t rbt;
    rbt_init(&rbt);

    int vals[7] = { 1, 3, 4, 7, 8, 13, 14 };
    for(int i = 0; i < 7; ++i) {
        rbt_node_t* node = rbt_malloc();
        int index = (i * 5) % 7;
        rbt_init_node(node, vals[index], vals[index]);
        rbt_insert(&rbt, node);
    }
    for(int i = 0; i < 7; ++i) {
        rbt_node_t* node = rbt_delete(&rbt, vals[7-(i+1)]);
        rbt_free(node);
        verify_rbt(&rbt, vals, 7 - (i + 1));
    }
}

void rbt_test() {
    printf("RBT TEST:\n");
    rbt_allocator = fbaa_new(malloc, MAX_NODES, sizeof(rbt_node_t));
    test_empty();
    test_singleton();
    test_many_insertions();
    test_delete();
    test_delete_all();
    printf("RBT TEST DONE\n");
    fbaa_destroy(free, rbt_allocator);
}

int main() {
    rbt_test();
}
