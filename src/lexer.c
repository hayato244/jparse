#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../include/helper.h"
#include "../include/lexer.h"
#include "../include/shared.h"

#define MAX_KEYWORD_LENGTH 5

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
        return JSON_TRUE_TOKEN;
    }

    if (strncmp(word, "false", MAX_KEYWORD_LENGTH) == 0)
    {
        return JSON_FALSE_TOKEN;
    }

    if (strncmp(word, "null", MAX_KEYWORD_LENGTH) == 0)
    {
        return JSON_NULL_TOKEN;
    }

    return JSON_NONE_TOKEN;
}

static void token_value(lexer_t *lexer, token_t *token, bool (*compare)(char))
{
    uint32_t read_chars = 0;
    token->value = lexer->token_values + lexer->token_values_offset;

    while (compare(reader_peek(lexer->reader)))
    {
        char next_char = reader_next(lexer->reader);

        token->value[read_chars] = next_char;
        read_chars++;
        lexer->token_values_offset++;
    }

    token->value[read_chars] = '\0';
    lexer->token_values_offset++;
}

static void syntax_token(lexer_t *lexer, token_t *token)
{
    char next_char = reader_next(lexer->reader);
    token->type = SYNTAX_TOKEN;

    switch (next_char)
    {
        case '{':
            token->json_token = JSON_LEFT_BRACE_TOKEN;
            break;

        case '}':
            token->json_token = JSON_RIGHT_BRACE_TOKEN;
            break;

        case '[':
            token->json_token = JSON_LEFT_BRACKET_TOKEN;
            break;

        case ']':
            token->json_token = JSON_RIGHT_BRACKET_TOKEN;
            break;

        case ',':
            token->json_token = JSON_COMMA_TOKEN;
            break;

        case ':':
            token->json_token = JSON_COLON_TOKEN;
            break;

        case '"':
            token->json_token = JSON_DOUBLE_QUOTE_TOKEN;
            break;
    }

    printf("[TOKEN] Type: SYNTAX   | %c\n", next_char);
}

// TODO: Enable escaped double quote character in string
static void string_token(lexer_t *lexer, token_t *token)
{
    reader_next(lexer->reader); // Consume first double quote

    token->type = LITERAL_TOKEN;
    token->json_token = JSON_STRING_TOKEN;

    token_value(lexer, token, string_compare);

    reader_next(lexer->reader); // Consume second double quote

    printf("[TOKEN] Type: LITERAL  | %s\n", token->value);
}

static void numeric_token(lexer_t *lexer, token_t *token)
{
    token->type = LITERAL_TOKEN;
    token->json_token = JSON_NUMBER_TOKEN;

    token_value(lexer, token, numeric_compare);

    printf("[TOKEN] Type: LITERAL  | %s\n", token->value);
}

static void keyword_token(lexer_t *lexer, token_t *token)
{
    token->type = KEYWORD_TOKEN;

    token_value(lexer, token, keyword_compare);

    token->json_token = determine_keyword(token->value);

    printf("[TOKEN] Type: KEYWORD  | %s\n", token->value);
}

static void eof_token(token_t *token)
{
    token->type = EOF_TOKEN;
    token->json_token = JSON_NONE_TOKEN;
}

static void skip_whitespace(reader_t *reader)
{
    while (is_whitespace(reader_peek(reader)))
    {
        reader_next(reader);
    }
}

static void fill(lexer_t *lexer)
{
    token_t token = {0};

    while (lexer->token_buffer_count < TOKEN_BUFFER_SIZE)
    {
        skip_whitespace(lexer->reader);

        char peeked_char = reader_peek(lexer->reader);

        if (is_numeric(peeked_char))
        {
            numeric_token(lexer, &token);
            goto fill;
        }

        if (is_alpha(peeked_char))
        {
            keyword_token(lexer, &token);
            goto fill;
        }

        switch (peeked_char)
        {
            case '{':
            case '}':
            case '[':
            case ']':
            case ',':
            case ':':
                syntax_token(lexer, &token);
                break;

            case '"':
                string_token(lexer, &token);
                break;

            case EOF:
                eof_token(&token);
                break;
        }

    fill:
        lexer->token_buffer[lexer->token_buffer_head] = token;
        lexer->token_buffer_head = (lexer->token_buffer_head + 1) % TOKEN_BUFFER_SIZE;
        lexer->token_buffer_count++;
    }
}

lexer_t lexer_init(reader_t *reader, arena_t *arena)
{
    lexer_t lexer = {0};

    lexer.token_values = (char *)ARENA_ALLOC_ARRAY(arena, KiB * 3, char);
    lexer.reader = reader;

    return lexer;
}

void lexer_peek(lexer_t *lexer, token_t *token)
{
    if (lexer->token_buffer_count == 0)
    {
        fill(lexer);
    }

    token->value = lexer->token_buffer[lexer->token_buffer_tail].value;
    token->json_token = lexer->token_buffer[lexer->token_buffer_tail].json_token;
    token->type = lexer->token_buffer[lexer->token_buffer_tail].type;
}

void lexer_next(lexer_t *lexer, token_t *token)
{
    if (lexer->token_buffer_count < TOKEN_BUFFER_SIZE)
    {
        fill(lexer);
    }

    token->value = lexer->token_buffer[lexer->token_buffer_tail].value;
    token->json_token = lexer->token_buffer[lexer->token_buffer_tail].json_token;
    token->type = lexer->token_buffer[lexer->token_buffer_tail].type;

    lexer->token_buffer_tail = (lexer->token_buffer_tail + 1) % TOKEN_BUFFER_SIZE;
    lexer->token_buffer_count--;
}
