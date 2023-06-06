#ifndef SGS_RING_BUFFER_H
#define SGS_RING_BUFFER_H

typedef struct {
    int front, back, size, max;
} ring_buffer_t;

void rb_clear(ring_buffer_t* buffer);
void rb_init(
    ring_buffer_t* buffer,
    int size
);
int rb_push_back(ring_buffer_t* buffer);
int rb_push_front(ring_buffer_t* buffer);
void rb_pop_back(ring_buffer_t* buffer);
void rb_pop_front(ring_buffer_t* buffer);
int rb_back(ring_buffer_t* buffer);
int rb_front(ring_buffer_t* buffer);
int rb_size(ring_buffer_t* buffer);

#endif // SGS_RING_BUFFER_H
