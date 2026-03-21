#include "arena.h"
#include "reader.h"

static void internal_fill(FILE *fp, reader_t *reader)
{
    char ch[1];

    while (fread(ch, sizeof(char), 1, fp))
    {
        if (reader->count == READER_BUFFER_SIZE)
        {
            break;
        }
    
        reader->buffer[reader->head] = *ch;
        reader->head = (reader->head + 1) % READER_BUFFER_SIZE;
        reader->count++;
    }
}

char reader_peek(FILE *fp, reader_t *reader)
{
    if (reader->count == 0)
    {
        internal_fill(fp, reader);
    }

    return reader->buffer[reader->tail];
}

char reader_next(FILE *fp, reader_t *reader)
{
    internal_fill(fp, reader);
    
    char ch = reader->buffer[reader->tail];
    reader->tail = (reader->tail + 1) % READER_BUFFER_SIZE;
    reader->count--;

    return ch;
}
