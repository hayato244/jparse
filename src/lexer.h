#include <stdint.h>

#include "arena.h"
#include "reader.h"

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

void create_syntax_token(token_stream_t *token_stream, token_t *token, reader_t *reader);

void create_string_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena);

void create_numeric_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena);

void create_keyword_token(token_stream_t *token_stream, token_t *token, reader_t *reader, arena_t *arena);

void create_eof_token(token_stream_t *token_stream, token_t *token);
