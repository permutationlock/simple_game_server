#include "ring_buffer.h"

void rb_clear(ring_buffer_t* buffer) {
    buffer->front = 0;
    buffer->back = 0;
    buffer->size = 0;
}

void rb_init(
    ring_buffer_t* buffer,
    int size
) {
    buffer->max = size;
    rb_clear(buffer);
}

int rb_push_back(ring_buffer_t* buffer) {
    if(buffer->size == buffer->max) {
        return -1;
    }

    int old_back = buffer->back;
    buffer->size += 1;
    buffer->back = (buffer->back + 1) % buffer->max;
    return old_back;
}

int rb_push_front(ring_buffer_t* buffer) {
    if(buffer->size == buffer->max) {
        return -1;
    }
    if(buffer->size == 0) {
        buffer->front = (buffer->front + 1) % buffer->max; 
    }

    buffer->size += 1;
    buffer->front = (buffer->front + (buffer->max - 1)) % buffer->max;
    return buffer->front;
}

void rb_pop_back(ring_buffer_t* buffer) {
    if(buffer->size == 0) {
        return;
    }

    buffer->back = (buffer->back + buffer->max - 1) % buffer->max;
    buffer->size -= 1;
}

void rb_pop_front(ring_buffer_t* buffer) {
    if(buffer->size == 0) {
       return;
    }

    buffer->front = (buffer->front + 1) % buffer->max;
    buffer->size -= 1;
}

int rb_back(ring_buffer_t* buffer) {
    return (buffer->back + buffer->max - 1) % buffer->max;
}

int rb_front(ring_buffer_t* buffer) {
    return buffer->front;
}

int rb_size(ring_buffer_t* buffer) {
    return buffer->size;
}
