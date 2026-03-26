#ifndef ARENA_H
#define ARENA_H

#include <string.h>
#include <stdint.h>

#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

#define IS_POWER_OF_TWO(alignment) ((alignment != 0) && ((alignment & (alignment - 1)) == 0))

#define ARENA_ALLOC_ARRAY(arena, size, type) (type *)(arena_alloc((arena), (size) * sizeof(type)))
#define ARENA_ALLOC_STRUCT(arena, type) ARENA_ALLOC_ARRAY((arena), 1, type)

struct arena
{
    char *base;

    size_t reserved_size;
    size_t offset;
};

typedef struct arena arena_t;

/**
 * Allocates an arena structure along with a contiguous
 * memory block of `size` bytes.
 * 
 * If the allocation fails, any previously allocated memory
 * is freed and NULL will be returned.
 */
arena_t *arena_init(size_t size);

/**
 * Bumps the `arena`'s offset forward by `size` bytes which is
 * automatically aligned. The returned memory is zero-initialized.
 * 
 * Returns NULL if the `arena` has insufficient space or if `arena` is
 * NULL or `size` is 0.
 */
void *arena_alloc(arena_t *arena, size_t size);

/**
 * Rolls back the `arena`'s offset by `size` bytes reclaiming the most
 * recently allocated block of memory.
 * 
 * If underflow would occur, the call is a no-op.
 */
void arena_free(arena_t *arena, size_t size);

/**
 * Sets the internal offset of `arena` back to zero, effectively freeing
 * all previously allocated memory. Previously returned pointers will be
 * invalidated after calling this function.
 */
void arena_reset(arena_t *arena);

/**
 * Releases the memory block managed by `arena` (if any),
 * followed by the `arena` structure itself.
 */
void arena_destroy(arena_t *arena);

#endif