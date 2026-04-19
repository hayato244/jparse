#ifndef PARSER_H
#define PARSER_H

#include "../include/arena.h"
#include "../include/lexer.h"

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
        struct {
            struct json_ast *next;
        } json_object;

        struct {
            struct json_ast *string;
            struct json_ast *value;
            struct json_ast *next;
        } json_object_key;

        struct {
            struct json_ast *next;
        } json_array;

        struct {
            struct json_ast *value;
            struct json_ast *next;
        } json_array_value;

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
