#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "lexer.h"

#define MAX_KEYWORD_LENGTH 5

static void append_token(token_stream_t *token_stream, token_t *token)
{
    memcpy(
        token_stream->tokens + token_stream->count, 
        token, 
        sizeof(token_t)
    );

    token_stream->count++;
}

static bool string_compare(char c)
{
    return c != '"';
}

static bool numeric_compare(char c)
{
    return is_numeric(c);
}

static bool keyword_compare(char c)
{
    return is_alpha(c);
}

static json_token determine_keyword(char *word)
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

// TODO: If value is exactly 16 chars long, no null char will be at the end
static void create_token_value(token_t *token, reader_t *reader, arena_t *arena, bool(*compare)(char))
{
    uint32_t read_chars = 0;
    char *value = (char *)arena_alloc(arena, sizeof(char));

    while (compare(reader_peek(reader)))
    {
        char next_char = reader_next(reader);
        char *p = value + read_chars;

        *p = next_char;
        read_chars++;

        // Allocate new space for proper alignment
        if (read_chars % DEFAULT_ALIGNMENT == 0)
        {
            read_chars = 0;
            value = (char *)arena_alloc(arena, sizeof(char));
        }
    }

    token->value = value;
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

// TODO: Enable escaped double quote character in string
void create_string_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena)
{
    reader_next(reader); // Consume first double quote

    token->token_type = LITERAL_TOKEN;
    token->json_token = JSON_STRING;

    create_token_value(token, reader, arena, string_compare);

    reader_next(reader); // Consume second double quote

    append_token(token_stream, token);
}

void create_numeric_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena)
{
    token->token_type = LITERAL_TOKEN;
    token->json_token = JSON_NUMBER;

    create_token_value(token, reader, arena, numeric_compare);

    append_token(token_stream, token);
}

void create_keyword_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena)
{
    token->token_type = KEYWORD_TOKEN;

    create_token_value(token, reader, arena, keyword_compare);

    token->json_token = determine_keyword(token->value);

    append_token(token_stream, token);
}

void create_eof_token(token_stream_t *token_stream, token_t *token)
{
    token->token_type = EOF_TOKEN;
    token->json_token = JSON_NONE;
    
    append_token(token_stream, token);
}
