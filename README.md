# Simple Game Server

A learning project making a game server in C from scratch.
The server was not completed, and won't currently build. I was working
very fast and loose, and the build system is simply shell scripts.

The data structures are the most complete and interesting part of this repo:

 - The `fbaa.*` files define a simple fixed capacity item pool allocator
(or "fixed-block array allocator" as I refered to it at the time).
It uses a ring array (aka a deque), defined in `ring_buffer.*`, to track
available space.

 - The `ring_fbaa.*` files define a fixed capacity item pool allocator that
also keeps an internal doubly-linked list, defined in `linked_list.*`, to
track the order that items were allocated. This allows old items to be
checked and deallocated in a system where this makes sense.

 - The `rbt.*` files define an allocater independent red-black tree implementation.
The `array_rbt.*` files use `fbaa` and `rbt` to define a red-black tree backed
by a fixed capacity item pool to allocate nodes.

This project moved to [Zig][1] and became the Zig game server [thierd][2].

[1]: https://ziglang.org
[2]: https://github.com/permutationlock/thierd
