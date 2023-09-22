#include "rbt.h"

void rbt_init(rbt_t* rbt) {
    rbt->size = 0;
    rbt->root = (rbt_node_t*)0;
}

void rbt_init_node(rbt_node_t* node, long key, int val) {
    node->left = (rbt_node_t*)0;
    node->right = (rbt_node_t*)0;
    node->color = RED;
    node->key = key;
    node->val = val;
}

rbt_node_t* rbt_insert_rec(
    rbt_node_t* node, rbt_node_t* new_node
) {
    if(node == (rbt_node_t*)0) {
        return new_node;
    }

    if(new_node->key < node->key) {
        node->left = rbt_insert_rec(node->left, new_node);
    } else {
        node->right = rbt_insert_rec(node->right, new_node);
    }

    return rbt_balance(node);
}

void rbt_insert(rbt_t* rbt, rbt_node_t* new_node) {
    rbt->root = rbt_insert_rec(rbt->root, new_node);
    rbt->size += 1;
}

rbt_node_t* rbt_find(rbt_t* rbt, long key) {
    rbt_node_t* node = rbt->root;
    while(node != (rbt_node_t*)0) {
        if(key == node->key) {
            break;
        } else if(key < node->key) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return node;
}

rbt_node_t* rbt_min(rbt_node_t* node) {
    while(node->left != (rbt_node_t*)0) {
        node = node->left;
    }
    return node;
}

rbt_node_t* rbt_max(rbt_node_t* node) {
    while(node->right != (rbt_node_t*)0) {
        node = node->right;
    }
    return node;
}

rbt_node_t* rbt_delete_min(rbt_node_t* node, rbt_node_t** min) {
    if(node->left == (rbt_node_t*)0) {
        *min = node;
        return (rbt_node_t*)0;
    }
    if(!rbt_is_red(node->left) && !rbt_is_red(node->left->left)) {
        node = rbt_move_red_left(node);
    }
    node->left = rbt_delete_min(node->left, min);
    return rbt_balance(node);
}

rbt_node_t* rbt_delete_rec(
    rbt_node_t* node, long key, rbt_node_t** dnode
) {
    if(key < node->key) {
        if(!rbt_is_red(node->left) && node->left != (rbt_node_t*)0) {
            if(!rbt_is_red(node->left->left)) {
                node = rbt_move_red_left(node);
            }
        }
        node->left = rbt_delete_rec(node->left, key, dnode);
    } else {
        if(rbt_is_red(node->left)) {
            node = rbt_rotate_right(node);
        }
        if(key == node->key && node->right == (rbt_node_t*)0) {
            *dnode = node;
            return node->left;
        }
        if(!rbt_is_red(node->right)
            && node->right != (rbt_node_t*)0
        ) {
            if(!rbt_is_red(node->right->left)) {
                node = rbt_move_red_right(node);
            }
        }
        if(key == node->key) {
            rbt_node_t* min;
            node->right = rbt_delete_min(node->right, &min);
            node->key = min->key;
            node->val = min->val;
            *dnode = min;
        } else {
            node->right = rbt_delete_rec(node->right, key, dnode);
        }
    } 

    return rbt_balance(node);
}

rbt_node_t* rbt_delete(rbt_t* rbt, long key) {
    //rbt_node_t* node = rbt_find(rbt, key);
    //if(node == (rbt_node_t*)0) {
    //    return (rbt_node_t*)0;
    //}
    rbt_node_t* dnode = (rbt_node_t*)0;
    rbt->root = rbt_delete_rec(rbt->root, key, &dnode);
    rbt->size -= 1;
    return dnode;
}

int rbt_is_red(rbt_node_t* node) {
    if(node == (rbt_node_t*)0) {
        return 0;
    }
    return node->color == RED;
}

rbt_node_t* rbt_rotate_left(rbt_node_t* node) {
    rbt_node_t* rc = node->right;
    node->right = rc->left;
    rc->left = node;
    rc->color = node->color;
    node->color = RED;
    return rc;
}

rbt_node_t* rbt_rotate_right(rbt_node_t* node) {
    rbt_node_t* lc = node->left;
    node->left = lc->right;
    lc->right = node;
    lc->color = node->color;
    node->color = RED;
    return lc;
}

void rbt_flip_colors(rbt_node_t* node) {
    node->color = ~(node->color);
    node->left->color = ~(node->left->color);
    node->right->color = ~(node->right->color);
}

rbt_node_t* rbt_move_red_left(rbt_node_t* node) {
    rbt_flip_colors(node);
    if(node->right != (rbt_node_t*)0) {
        if(rbt_is_red(node->right->left)) {
            node->right = rbt_rotate_right(node->right);
            node = rbt_rotate_left(node);
            rbt_flip_colors(node);
        }
    }
    return node;
}

rbt_node_t* rbt_move_red_right(rbt_node_t* node) {
    rbt_flip_colors(node);
    if(node->left != (rbt_node_t*)0) {
        if(rbt_is_red(node->left->left)) {
            node = rbt_rotate_right(node);
            rbt_flip_colors(node);
        }
    }
    return node;
}

rbt_node_t* rbt_balance(rbt_node_t* node) { 
    if(rbt_is_red(node->right)) {
        node = rbt_rotate_left(node);
    }
    if(rbt_is_red(node->left)) {
        if(rbt_is_red(node->left->left)) {
            node = rbt_rotate_right(node);
        }
    }
    if(rbt_is_red(node->left) && rbt_is_red(node->right)) {
        rbt_flip_colors(node);
    }
    return node;
}
