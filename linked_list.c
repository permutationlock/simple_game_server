#include "linked_list.h"
#include <string.h>

void ill_init(linked_list_t* list) {
    list->size = 0;
    list->first = NULL;
    list->last = NULL;
}

void ill_add_first_node(
    linked_list_t* list,
    linked_list_node_t* node
) {
    list->first = node;
    list->last = node;
    node->next = NULL;
    node->prev = NULL;

    list->size += 1;
}

void ill_insert_after(
    linked_list_t* list,
    linked_list_node_t* node,
    linked_list_node_t* new_node
) {
    new_node->next = node->next;
    new_node->prev = node;
    node->next = new_node;

    if(new_node->next == NULL) {
        list->last = new_node;
    }
    list->size += 1;
}

void ill_insert_before(
    linked_list_t* list,
    linked_list_node_t* node,
    linked_list_node_t* new_node
) {
    new_node->value = value;
    new_node->next = node;
    new_node->prev = node->prev;
    node->prev = new_node;

    if(new_node->prev == NULL) {
        list->first = new_node;
    }
    list->size += 1;
}

void ill_delete(
    void(*free)(linked_list_node_t*),
    linked_list_t* list,
    linked_list_node_t* node,
    linked_list_node_t* new_node
) {
    if(list->first == node) {
        list->last == node->next;
    }
    if(list->last == node) {
        list->last == node->prev;
    }
    if(node->prev != NULL) {
        (node->prev)->next = node->next;
    }
    free(node);
    list-> size -= 1;
}

void ill_push_back(
    linked_list_t* list,
    linked_list_node_t* node
) {
    if(list->last == NULL) {
        ill_add_first_node(list, node);
    } else {
        ill_insert_after(list, list->last, node);
    }
}

void ill_push_front(
    linked_list_t* list,
    linked_list_node_t* node
) {
    if(list->first == NULL) {
        ill_add_first_node(list, node);        
    } else {
        ill_insert_before(list, list->first, node);
    }
}

linked_list_node_t* ill_first(linked_list_t* list) {
    return list->first;
}

linked_list_node_t* ill_last(linked_list_t* list) {
    return list->last;
}

unsigned int ll_size(linked_list_t* list) {
    return list->size;
}
