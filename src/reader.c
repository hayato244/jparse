#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/arena.h"
#include "../include/reader.h"

static void fill(reader_t *reader)
{
    char ch;

    while ((ch = fgetc(reader->fp)))
    {
        reader->buffer[reader->head] = (reader_char_t){
            .ch = ch,
            .column = reader->column,
            .line = reader->line,
        };

        reader->head = (reader->head + 1) % READER_BUFFER_SIZE;
        reader->count++;

        reader->column++;

        if (ch == '\n')
        {
            reader->line++;
            reader->column = 1;
        }

        if (reader->count == READER_BUFFER_SIZE)
        {
            break;
        }
    }
}

reader_t reader_init(const char *path)
{
    reader_t reader = {0};

    reader.line = 1;
    reader.column = 1;

    if ((reader.fp = fopen(path, "r")) == NULL)
    {
        perror("fopen");
        exit(1);
    }

    return reader;
}

reader_char_t reader_peek(reader_t *reader)
{
    if (reader->count == 0)
    {
        fill(reader);
    }

    return reader->buffer[reader->tail];
}

reader_char_t reader_peekn(reader_t *reader, int32_t n)
{
    assert(n < READER_BUFFER_SIZE);
    assert(n > -2); // Allows looking at the previous character

    if (reader->count == 0)
    {
        fill(reader);
    }

    int tail = reader->tail;
    int lookup = (tail + n);

    return reader->buffer[lookup < 0 ? READER_BUFFER_SIZE - 1 : lookup];
}

reader_char_t reader_next(reader_t *reader)
{
    if (reader->count < READER_BUFFER_SIZE)
    {
        fill(reader);
    }

    reader_char_t rc = reader->buffer[reader->tail];
    reader->tail = (reader->tail + 1) % READER_BUFFER_SIZE;
    reader->count--;

    return rc;
}
