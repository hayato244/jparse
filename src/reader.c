#include <assert.h>

#include "arena.h"
#include "reader.h"

static void fill(reader_t *reader)
{
    char ch;

    while ((ch = fgetc(reader->fp)))
    {
        reader->buffer[reader->head] = ch;
        reader->head = (reader->head + 1) % READER_BUFFER_SIZE;
        reader->count++;
        
        if (reader->count == READER_BUFFER_SIZE)
        {
            break;
        }
    }
}

char reader_peek(reader_t *reader)
{
    if (reader->count == 0)
    {
        fill(reader);
    }

    return reader->buffer[reader->tail];
}

char reader_peekn(reader_t *reader, int32_t n)
{
    assert(n < READER_BUFFER_SIZE);
    
    if (reader->count == 0)
    {
        fill(reader);
    }

    return reader->buffer[reader->tail + n];
}

char reader_next(reader_t *reader)
{
    if (reader->count < READER_BUFFER_SIZE)
    {
        fill(reader);
    }

    char ch = reader->buffer[reader->tail];
    reader->tail = (reader->tail + 1) % READER_BUFFER_SIZE;
    reader->count--;

    return ch;
}
