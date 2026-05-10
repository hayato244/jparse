#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../include/helper.h"
#include "../include/lexer.h"
#include "../include/shared.h"

#define MAX_KEYWORD_LENGTH 5

static bool continue_string(reader_t *reader)
{
    char c = reader_peek(reader).ch;
    char prev_char = reader_peekn(reader, -1).ch;

    if (c == '"')
    {
        return prev_char == '\\';
    }

    return true;
}

static bool continue_number(reader_t *reader)
{
    return is_numeric(reader_peek(reader).ch);
}

static bool continue_keyword(reader_t *reader)
{
    return is_alpha(reader_peek(reader).ch);
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

static void token_value(lexer_t *lexer, token_t *token, bool (*cont)(reader_t *))
{
    uint32_t read_chars = 0;
    token->value = lexer->token_values + lexer->token_values_offset;

    while (cont(lexer->reader))
    {
        char next_char = reader_next(lexer->reader).ch;

        token->value[read_chars] = next_char;
        read_chars++;
        lexer->token_values_offset++;
    }

    token->value[read_chars] = '\0';
    lexer->token_values_offset++;
}

static void syntax_token(lexer_t *lexer, token_t *token)
{
    reader_char_t next_rc = reader_next(lexer->reader);

    token->type = SYNTAX_TOKEN;

    switch (next_rc.ch)
    {
        case '{': token->json_token = JSON_LEFT_BRACE_TOKEN; break;
        case '}': token->json_token = JSON_RIGHT_BRACE_TOKEN; break;
        case '[': token->json_token = JSON_LEFT_BRACKET_TOKEN; break;
        case ']': token->json_token = JSON_RIGHT_BRACKET_TOKEN; break;
        case ',': token->json_token = JSON_COMMA_TOKEN; break;
        case ':': token->json_token = JSON_COLON_TOKEN; break;
        case '"': token->json_token = JSON_DOUBLE_QUOTE_TOKEN; break;
    }

    printf("Type: syntax  | Line: %3u, Col: %3u | %c\n", next_rc.line, next_rc.column, next_rc.ch);

    token->line = next_rc.line;
    token->column = next_rc.column;
}

static void string_token(lexer_t *lexer, token_t *token)
{
    reader_char_t peeked_rc = reader_peek(lexer->reader);

    reader_next(lexer->reader); // Consume first double quote

    token->type = LITERAL_TOKEN;
    token->json_token = JSON_STRING_TOKEN;

    token_value(lexer, token, continue_string);

    reader_next(lexer->reader); // Consume second double quote

    printf("Type: literal | Line: %3u, Col: %3u | %s\n", peeked_rc.line, peeked_rc.column, token->value);

    token->line = peeked_rc.line;
    token->column = peeked_rc.column;
}

static void numeric_token(lexer_t *lexer, token_t *token)
{
    reader_char_t peeked_rc = reader_peek(lexer->reader);

    token->type = LITERAL_TOKEN;
    token->json_token = JSON_NUMBER_TOKEN;

    token_value(lexer, token, continue_number);

    printf("Type: literal | Line: %3u, Col: %3u | %s\n", peeked_rc.line, peeked_rc.column, token->value);

    token->line = peeked_rc.line;
    token->column = peeked_rc.column;
}

static void keyword_token(lexer_t *lexer, token_t *token)
{
    reader_char_t peeked_rc = reader_peek(lexer->reader);

    token->type = KEYWORD_TOKEN;

    token_value(lexer, token, continue_keyword);

    token->json_token = determine_keyword(token->value);

    printf("Type: keyword | Line: %3u, Col: %3u | %s\n", peeked_rc.line, peeked_rc.column, token->value);

    token->line = peeked_rc.line;
    token->column = peeked_rc.column;
}

static void eof_token(token_t *token)
{
    token->type = EOF_TOKEN;
    token->json_token = JSON_NONE_TOKEN;
}

static void skip_whitespace(reader_t *reader)
{
    while (is_whitespace(reader_peek(reader).ch))
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

        char peeked_char = reader_peek(lexer->reader).ch;

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
    token->line = lexer->token_buffer[lexer->token_buffer_tail].line;
    token->column = lexer->token_buffer[lexer->token_buffer_tail].column;
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
    token->line = lexer->token_buffer[lexer->token_buffer_tail].line;
    token->column = lexer->token_buffer[lexer->token_buffer_tail].column;

    lexer->token_buffer_tail = (lexer->token_buffer_tail + 1) % TOKEN_BUFFER_SIZE;
    lexer->token_buffer_count--;
}
