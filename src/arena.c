#include <stdlib.h>
#include <string.h>

#include "../include/arena.h"

arena_t *arena_init(size_t size)
{
    arena_t *arena = malloc(sizeof(arena_t));

    if (arena == NULL)
    {
        return NULL;
    }

    void *chunk = malloc(size);

    if (chunk == NULL)
    {
        free(arena);
        return NULL;
    }

    arena->base = (char *)chunk;
    arena->reserved_size = size;
    arena->offset = 0;

    return arena;
}

static void *arena_alloc_aligned(arena_t *arena, size_t size, size_t alignment)
{
    if (alignment == 0 || !IS_POWER_OF_TWO(alignment))
    {
        return NULL;
    }

    if (arena == NULL || size == 0)
    {
        return NULL;
    }

    // TODO: Implement reallocating buffer

    if (size > arena->reserved_size)
    {
        return NULL;
    }

    size_t aligned_offset = (arena->offset + alignment - 1) & ~(alignment - 1);

    if (size + aligned_offset > arena->reserved_size)
    {
        return NULL;
    }

    char *free_addr = arena->base + aligned_offset;
    memset(free_addr, 0, size);

    arena->offset = aligned_offset + size;

    return (void *)free_addr;
}

void *arena_alloc(arena_t *arena, size_t size)
{
    return arena_alloc_aligned(arena, size, DEFAULT_ALIGNMENT);
}

void arena_reset(arena_t *arena)
{
    if (arena == NULL)
    {
        return;
    }

    arena->offset = 0;
}

void arena_free(arena_t *arena, size_t size)
{
    if (arena == NULL || size == 0)
    {
        return;
    }

    // TODO: Maybe reset arena instead of returning???
    if ((int64_t)arena->offset - (int64_t)size < 0)
    {
        return;
    }

    arena->offset -= size;
}

void arena_destroy(arena_t *arena)
{
    if (arena == NULL)
    {
        return;
    }

    if (arena->base == NULL)
    {
        free(arena);
        return;
    }

    free(arena->base);
    free(arena);
}
