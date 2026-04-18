#ifndef PARSER_H
#define PARSER_H

#include "arena.h"
#include "lexer.h"

typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_OBJECT_KEY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL
} json_tag;

typedef struct json_ast {
    json_tag tag;
    union {
        struct json_object {
            struct json_ast *next;
        } json_object;

        struct json_object_key {
            struct json_ast *string;
            struct json_ast *value;
            struct json_ast *next;
        } json_object_key;

        struct json_array {
            struct json_ast *next;
        } json_array;

        char *string;
        float number;
        bool boolean;
        void *null;
    };
} json_ast_t;

typedef struct {
    lexer_t *lexer;
    json_ast_t *json_ast;
} parser_t;

static json_ast_t *parse_json_object(parser_t *parser, arena_t *arena);

static json_ast_t *parse_json_array(parser_t *parser, arena_t *arena);

#endif
