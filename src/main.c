#include <stdint.h>
#include <stdio.h>

#include "arena.h"
#include "helper.h"
#include "reader.h"

#define KiB (1 << 10)
#define MiB (KiB * KiB)

#define MAX_KEYWORD_LENGTH 5

typedef enum {
    KEYWORD_TOKEN,
    LITERAL_TOKEN,
    SYNTAX_TOKEN,
    EOF_TOKEN,
} token_type;

typedef enum {
    JSON_NONE, // Basically EOF
    JSON_LEFT_BRACE,
    JSON_RIGHT_BRACE,
    JSON_LEFT_BRACKET,
    JSON_RIGHT_BRACKET,
    JSON_DOUBLE_QUOTE,
    JSON_COLON,
    JSON_COMMA,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL,
    JSON_STRING,
    JSON_NUMBER,
} json_token;

typedef struct {
    token_type token_type;
    json_token json_token;
    char *value;
} token_t;

typedef struct {
    uint32_t count;
    token_t *tokens;
} token_stream_t;

void append_token(token_stream_t *token_stream, token_t *token)
{
    memcpy(
        token_stream->tokens + token_stream->count, 
        token, 
        sizeof(token_t)
    );

    token_stream->count++;
}

json_token determine_keyword(char *word)
{
    if (strncmp(word, "true", MAX_KEYWORD_LENGTH) == 0)
    {
        return JSON_TRUE;
    }

    if (strncmp(word, "false", MAX_KEYWORD_LENGTH) == 0)
    {
        return JSON_FALSE;
    }

    if (strncmp(word, "null", MAX_KEYWORD_LENGTH) == 0)
    {
        return JSON_NULL;
    }

    return JSON_NONE;
}

void create_syntax_token(token_stream_t *token_stream, token_t *token, reader_t *reader)
{
    char next_char = reader_next(reader);
    token->token_type = SYNTAX_TOKEN;

    switch (next_char)
    {
        case '{': 
            token->json_token = JSON_LEFT_BRACE;
            break;
        
        case '}':
            token->json_token = JSON_RIGHT_BRACE;
            break;
            
        case '[':
            token->json_token = JSON_LEFT_BRACKET;
            break;
            
        case ']':
            token->json_token = JSON_RIGHT_BRACKET;
            break;
            
        case ',':
            token->json_token = JSON_COMMA;
            break;
            
        case ':':
            token->json_token = JSON_COLON;  
            break;  

        case '"':
            token->json_token = JSON_DOUBLE_QUOTE;
            break;
    }

    append_token(token_stream, token);
}

void create_string_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena)
{
    uint32_t read_chars = 0;
    char *literal = (char *)arena_alloc(arena, sizeof(char));

    token->token_type = LITERAL_TOKEN;
    token->json_token = JSON_STRING;

    // TODO: If string is exactly 16 chars long, no null char will be added to the end

    while (reader_peek(reader) != '"')
    {
        char next_char = reader_next(reader);

        memcpy(literal + read_chars, &next_char, sizeof(char));
        read_chars++;

        // Allocate new space for proper alignment
        if (read_chars % DEFAULT_ALIGNMENT == 0)
        {
            read_chars = 0;
            literal = (char *)arena_alloc(arena, sizeof(char));
        }
    }

    reader_next(reader); // Consume second double quote
    
    token->value = literal;
    append_token(token_stream, token);
}

void create_numeric_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena)
{
    uint32_t read_chars = 0;
    char *literal = (char *)arena_alloc(arena, sizeof(char));

    token->token_type = LITERAL_TOKEN;
    token->json_token = JSON_NUMBER;

    // TODO: If string is exactly 16 chars long, no null char will be at the end

    while (is_numeric(reader_peek(reader)))
    {
        char next_char = reader_next(reader);
        char *p = literal + read_chars;

        *p = next_char;
        read_chars++;

        // Allocate new space for proper alignment
        if (read_chars % DEFAULT_ALIGNMENT == 0)
        {
            read_chars = 0;
            literal = (char *)arena_alloc(arena, sizeof(char));
        }
    }

    token->value = literal;
    append_token(token_stream, token);
}

void create_keyword_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena)
{
    uint32_t read_chars = 0;
    char *keyword = (char *)arena_alloc(arena, sizeof(char));

    token->token_type = KEYWORD_TOKEN;

    // TODO: If string is exactly 16 chars long, no null char will be added to the end

    while (is_alpha(reader_peek(reader)))
    {
        char next_char = reader_next(reader);
        char *p = keyword + read_chars;

        *p = next_char;
        read_chars++;

        // Allocate new space for proper alignment
        if (read_chars % DEFAULT_ALIGNMENT == 0)
        {
            read_chars = 0;
            keyword = (char *)arena_alloc(arena, sizeof(char));
        }
    }

    token->json_token = determine_keyword(keyword);
    token->value = keyword;

    append_token(token_stream, token);
}

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
    
    if ((reader.fp = fopen(path, "r")) == NULL)
    {
        perror("fopen");
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
                token.token_type = EOF_TOKEN;
                token.json_token = JSON_NONE;
                append_token(&token_stream, &token);

                goto end;
        }
    }

end:
    printf("\nCreated a total of %u tokens\n", token_stream.count);
    printf("\n-- Finished lexing %s --\n", path);
    return 0;
}
