#ifndef SGS_RING_BUFFER_H
#define SGS_RING_BUFFER_H

typedef struct {
    unsigned int front, back, size, max;
    unsigned int* data;
} int_ring_buffer_t;

typedef struct {
    unsigned int front, back, size, max;
} ring_buffer_t;

void irb_clear(int_ring_buffer_t* buffer);
void irb_init(
    int_ring_buffer_t* buffer,
    unsigned int* mem,
    unsigned int size
);
void irb_push_back(int_ring_buffer_t* buffer, unsigned int element);
void irb_pop_back(int_ring_buffer_t* buffer);
void irb_pop_front(int_ring_buffer_t* buffer);
unsigned int irb_back(int_ring_buffer_t* buffer);
unsigned int irb_front(int_ring_buffer_t* buffer);
unsigned int irb_size(int_ring_buffer_t* buffer);

void rb_clear(ring_buffer_t* buffer);
void rb_init(
    ring_buffer_t* buffer,
    unsigned int size
);
unsigned int rb_push_back(ring_buffer_t* buffer);
void rb_pop_back(ring_buffer_t* buffer);
void rb_pop_front(ring_buffer_t* buffer);
unsigned int rb_back(ring_buffer_t* buffer);
unsigned int rb_front(ring_buffer_t* buffer);
unsigned int rb_size(ring_buffer_t* buffer);

#endif // SGS_RING_BUFFER_H
