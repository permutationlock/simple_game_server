#ifndef SGS_LINKED_LIST_H
#define SGS_LINKED_LIST_H

typedef struct ll_node_t {
    struct ll_node_t* prev;
    struct ll_node_t* next;
} ll_node_t;

typedef struct {
    unsigned int size;
    ll_node_t* first;
    ll_node_t* last;
} linked_list_t;

void ll_init(linked_list_t* list);
void ll_add_first_node(
    linked_list_t* list,
    ll_node_t* node
);
void ll_insert_after(
    linked_list_t* list,
    ll_node_t* node,
    ll_node_t* new_node
);
void ll_insert_before(
    linked_list_t* list,
    ll_node_t* node,
    ll_node_t* new_node
);
void ll_delete(
    linked_list_t* list,
    ll_node_t* node
);
void ll_push_back(
    linked_list_t* list,
    ll_node_t* node
);
void ll_push_front(
    linked_list_t* list,
    ll_node_t* node
);
ll_node_t* ll_first(linked_list_t* list);
ll_node_t* ll_last(linked_list_t* list);
unsigned int ll_size(linked_list_t* list);

#endif //SGS_LINKED_LIST_H
