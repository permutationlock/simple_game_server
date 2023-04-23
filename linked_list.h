#ifndef SGS_LINKED_LIST_H
#define SGS_LINKED_LIST_H

typedef struct {
    linked_list_node_t* prev, next;
    void* value;
} linked_list_node_t;

typedef struct {
    unsigned int size;
    int_linked_list_node_t first, last;
} linked_list_t;

void ll_init(linked_list_t* list);
void ll_add_first_node(
    linked_list_t* list,
    linked_list_node_t* node
);
void ll_insert_after(
    linked_list_t* list,
    linked_list_node_t* node,
    linked_list_node_t* new_node
);
void ll_insert_before(
    linked_list_t* list,
    linked_list_node_t* node,
    linked_list_node_t* new_node
);
void ll_delete(
    void(*free)(linked_list_node_t*),
    linked_list_t* list,
    linked_list_node_t* node,
    linked_list_node_t* new_node
);
void ll_push_back(
    linked_list_t* list,
    linked_list_node_t* node
);
void ll_push_front(
    linked_list_t* list,
    linked_list_node_t* node
);
linked_list_node_t* ll_first(linked_list_t* list);
linked_list_node_t* ll_last(linked_list_t* list);
unsigned int ll_size(linked_list_t* list);

#endif //SGS_LINKED_LIST_H
