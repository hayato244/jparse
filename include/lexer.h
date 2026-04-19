#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

#define TOKEN_BUFFER_SIZE 2

#include "../include/arena.h"
#include "../include/reader.h"

typedef enum {
    KEYWORD_TOKEN,
    LITERAL_TOKEN,
    SYNTAX_TOKEN,
    EOF_TOKEN,
} token_type;

typedef enum {
    JSON_NONE_TOKEN, // Basically EOF
    JSON_LEFT_BRACE_TOKEN,
    JSON_RIGHT_BRACE_TOKEN,
    JSON_LEFT_BRACKET_TOKEN,
    JSON_RIGHT_BRACKET_TOKEN,
    JSON_DOUBLE_QUOTE_TOKEN,
    JSON_COLON_TOKEN,
    JSON_COMMA_TOKEN,
    JSON_TRUE_TOKEN,
    JSON_FALSE_TOKEN,
    JSON_NULL_TOKEN,
    JSON_STRING_TOKEN,
    JSON_NUMBER_TOKEN,
} json_token;

typedef struct {
    token_type type;
    json_token json_token;
    char *value;
} token_t;

typedef struct {
    reader_t *reader;

    token_t token_buffer[TOKEN_BUFFER_SIZE];
    uint32_t token_buffer_head;
    uint32_t token_buffer_tail;
    uint32_t token_buffer_count;

    char *token_values;
    uint32_t token_values_offset;
} lexer_t;

lexer_t lexer_init(reader_t *reader, arena_t *arena);

void lexer_peek(lexer_t *lexer, token_t *token);

void lexer_next(lexer_t *lexer, token_t *token);

#endif
