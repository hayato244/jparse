#ifndef READER_H
#define READER_H

#include <stdint.h>
#include <stdio.h>

#include "arena.h"

#define READER_BUFFER_SIZE 5

typedef struct
{
    char buffer[READER_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} reader_t;

char reader_peek(FILE *fp, reader_t *reader);

char reader_next(FILE *fp, reader_t *reader);

#endif