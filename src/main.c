#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "helper.h"
#include "lexer.h"
#include "reader.h"

#define KiB (1 << 10)
#define MiB (KiB * KiB)

int main(int argc, char *argv[])
{
    reader_t reader = {0};
    const char *path = argv[1];
    
    if (argc != 2)
    {
    printf(
        "Usage: jparse <file>\n"
        "Parse JSON provided by <file>.\n");
        
        return 1;
    }
    
    if (!reader_init(&reader, path))
    {
        perror("reader_init");
        return 1;
    }

    printf("-- Begin lexing %s --\n\n", path);

    arena_t *token_arena = arena_init(sizeof(token_t) * 512);
    arena_t *value_arena = arena_init(MiB);

    token_stream_t token_stream;
    token_stream.tokens = (token_t *)ARENA_ALLOC_ARRAY(token_arena, 512, token_t);

    for (;;)
    {
        char peeked_char = reader_peek(&reader);
        char org_char = peeked_char;

        token_t token;
        token.value = NULL;

        if (is_whitespace(peeked_char))
        {
            reader_next(&reader);
            continue;
        }

        if (is_numeric(peeked_char))
        {
            create_numeric_token(&token_stream, &token, &reader, value_arena);
            printf("[TOKEN] Type: LITERAL  | %s\n", token.value);

            continue;
        }

        if (is_alpha(peeked_char))
        {
            create_keyword_token(&token_stream, &token, &reader, value_arena);
            printf("[TOKEN] Type: KEYWORD  | %s\n", token.value);

            continue;
        }

        switch (peeked_char)
        {
            case '{':
            case '}':
            case '[':
            case ']':
            case ',':
            case ':':
                create_syntax_token(&token_stream, &token, &reader);
                printf("[TOKEN] Type: SYNTAX   | %c\n", org_char);

                break;

            case '"':
                create_string_token(&token_stream, &token, &reader, value_arena);
                printf("[TOKEN] Type: LITERAL  | %s\n", token.value);

                break;
            
            case EOF:
                create_eof_token(&token_stream, &token);
                goto end;
        }
    }

end:
    printf("\nCreated a total of %u tokens\n", token_stream.count);
    printf("\n-- Finished lexing %s --\n", path);

    return 0;
}
