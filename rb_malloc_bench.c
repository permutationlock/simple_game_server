void allocate_blocks(
    void*(*malloc)(),
    void** blocks,
    int size
) {
    for(int i = 0; i < size; ++i) {
        blocks[i] = malloc();
    }
}

void free_blocks(
    void(*free)(void* p),
    void** blocks,
    int size
) {
    for(int i = 0; i < size; ++i) {
        free(blocks[i]);
    }
}
