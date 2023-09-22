#ifndef SGS_RBT_H
#define SGS_RBT_H

enum rbt_color_t {
    RED = 0,
    BLACK = 0xffffffff
};

typedef struct rbt_node_t {
    struct rbt_node_t* right;
    struct rbt_node_t* left;
    long key;
    int val;
    enum rbt_color_t color;
} rbt_node_t;

typedef struct {
    int size;
    rbt_node_t* root;
} rbt_t;

void rbt_init(rbt_t* rbt);
void rbt_init_node(
    rbt_node_t* node, long key, int val
);
void rbt_insert(rbt_t* rbt, rbt_node_t* new_node);
rbt_node_t* rbt_find(rbt_t* rbt, long key);
rbt_node_t* rbt_delete(rbt_t* rbt, long key);
rbt_node_t* rbt_min(rbt_node_t* node);
rbt_node_t* rbt_max(rbt_node_t* node);
int rbt_is_red(rbt_node_t* node);

rbt_node_t* rbt_rotate_left(rbt_node_t* node);
rbt_node_t* rbt_rotate_right(rbt_node_t* node);
void rbt_flip_color(rbt_node_t* node);
rbt_node_t* rbt_move_red_left(rbt_node_t* node);
rbt_node_t* rbt_move_red_right(rbt_node_t* node);
rbt_node_t* rbt_balance(rbt_node_t* node);

#endif // SGS_RBT_H
