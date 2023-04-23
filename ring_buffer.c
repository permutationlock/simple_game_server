#include "ring_buffer.h"

void irb_clear(int_ring_buffer_t* buffer) {
    buffer->front = 0;
    buffer->back = 0;
    buffer->size = 0;
}

void irb_init(
    int_ring_buffer_t* buffer,
    unsigned int* mem,
    unsigned int size
) {
    buffer->data = mem;
    buffer->max = size;
    irb_clear(buffer);
}

void irb_push_back(int_ring_buffer_t* buffer, unsigned int element) {
    if(buffer->size == buffer->max) {
        return;
    }

    buffer->data[buffer->back] = element;
    buffer->size += 1;
    buffer->back = (buffer->back + 1) % buffer->max;
}

void irb_pop_back(int_ring_buffer_t* buffer) {
    if(buffer->size == 0) {
        return;
    }

    buffer->back = (buffer->back + buffer->max - 1) % buffer->max;
    buffer->size -= 1;
}

void irb_pop_front(int_ring_buffer_t* buffer) {
    if(buffer->size == 0) {
        return;
    }

    buffer->front = (buffer->front + 1) % buffer->max;
    buffer->size -= 1;
}

unsigned int irb_back(int_ring_buffer_t* buffer) {
    return buffer->data[
        (buffer->back + buffer->max - 1) % buffer->max
    ];
}

unsigned int irb_front(int_ring_buffer_t* buffer) {
    return buffer->data[buffer->front];
}

unsigned int irb_size(int_ring_buffer_t* buffer) {
    return buffer->size;
}

void rb_clear(ring_buffer_t* buffer) {
    buffer->front = 0;
    buffer->back = 0;
    buffer->size = 0;
}

void rb_init(
    ring_buffer_t* buffer,
    unsigned int size
) {
    buffer->max = size;
    rb_clear(buffer);
}

unsigned int rb_push_back(ring_buffer_t* buffer) {
    if(buffer->size == buffer->max) {
        return 0;
    }

    unsigned int old_back = buffer->back;
    buffer->size += 1;
    buffer->back = (buffer->back + 1) % buffer->max;
    return old_back;
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

unsigned int rb_back(ring_buffer_t* buffer) {
    return (buffer->back + buffer->max - 1) % buffer->max;
}

unsigned int rb_front(ring_buffer_t* buffer) {
    return buffer->front;
}

unsigned int rb_size(ring_buffer_t* buffer) {
    return buffer->size;
}
