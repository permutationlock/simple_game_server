#include "linked_list.h"

void ll_init(linked_list_t* list) {
    list->size = 0;
    list->first = (ll_node_t*)0;
    list->last = (ll_node_t*)0;
}

void ll_add_first_node(
    linked_list_t* list,
    ll_node_t* node
) {
    list->first = node;
    list->last = node;
    node->next = (ll_node_t*)0;
    node->prev = (ll_node_t*)0;

    list->size += 1;
}

void ll_insert_after(
    linked_list_t* list,
    ll_node_t* node,
    ll_node_t* new_node
) {
    if(node->next != (ll_node_t*)0) {
        node->next->prev = new_node;
    }
    else {
        list->last = new_node;
    }

    new_node->next = node->next;
    new_node->prev = node;
    node->next = new_node;

    list->size += 1;
}

void ll_insert_before(
    linked_list_t* list,
    ll_node_t* node,
    ll_node_t* new_node
) {
    if(node->prev != (ll_node_t*)0) {
        node->prev->next = new_node;
    }
    else {
        list->first = new_node;
    }

    new_node->next = node;
    new_node->prev = node->prev;
    node->prev = new_node;

    list->size += 1;
}

void ll_delete(
    linked_list_t* list,
    ll_node_t* node
) {
    if(node->prev != (ll_node_t*)0) {
        (node->prev)->next = node->next;
    } else {
        list->first = node->next;
    }
    if(node->next != (ll_node_t*)0) {
        (node->next)->prev = node->prev;
    } else {
        list->last = node->prev;
    }
    list-> size -= 1;
}

void ll_push_back(
    linked_list_t* list,
    ll_node_t* node
) {
    if(list->last == (ll_node_t*)0) {
        ll_add_first_node(list, node);
    } else {
        ll_insert_after(list, list->last, node);
    }
}

void ll_push_front(
    linked_list_t* list,
    ll_node_t* node
) {
    if(list->first == (ll_node_t*)0) {
        ll_add_first_node(list, node);        
    } else {
        ll_insert_before(list, list->first, node);
    }
}

ll_node_t* ll_first(linked_list_t* list) {
    return list->first;
}

ll_node_t* ll_last(linked_list_t* list) {
    return list->last;
}

unsigned int ll_size(linked_list_t* list) {
    return list->size;
}
