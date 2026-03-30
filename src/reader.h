#ifndef READER_H
#define READER_H

#include <stdint.h>
#include <stdio.h>

#include "arena.h"

#define READER_BUFFER_SIZE 5

typedef struct
{
    FILE *fp;
    char buffer[READER_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} reader_t;

bool reader_init(reader_t *reader, const char *path);

char reader_peek(reader_t *reader);

char reader_peekn(reader_t *reader, int32_t n);

char reader_next(reader_t *reader);

#endif