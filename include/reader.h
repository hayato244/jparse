#ifndef READER_H
#define READER_H

#include <stdint.h>
#include <stdio.h>

#include "../include/arena.h"

#define READER_BUFFER_SIZE 3

typedef struct {
    uint16_t line;
    uint16_t column;

    char ch;
} reader_char_t;

typedef struct {
    FILE *fp;
    reader_char_t buffer[READER_BUFFER_SIZE];

    uint32_t head;
    uint32_t tail;
    uint32_t count;

    uint32_t line;
    uint32_t column;
} reader_t;

reader_t reader_init(const char *path);

reader_char_t reader_peek(reader_t *reader);

reader_char_t reader_peekn(reader_t *reader, int32_t n);

reader_char_t reader_next(reader_t *reader);

#endif
