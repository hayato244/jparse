#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/arena.h"
#include "../include/helper.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/reader.h"
#include "../include/shared.h"

json_ast_t *make_string_node(const token_t *token, arena_t *arena)
{
    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_STRING;
    node->string = token->value;

    return node;
}

json_ast_t *make_number_node(const token_t *token, arena_t *arena)
{
    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_NUMBER;

    // TODO: Implement own number parsing function
    node->number = strtof(token->value, NULL);

    return node;
}

json_ast_t *make_boolean_node(const token_t *token, arena_t *arena)
{
    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_BOOLEAN;
    node->boolean = token->json_token == JSON_TRUE_TOKEN ? true : false;

    return node;
}

json_ast_t *make_null_node(arena_t *arena)
{
    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_NULL;
    node->null = NULL;

    return node;
}

json_ast_t *make_object_node(json_ast_t *next, arena_t *arena)
{
    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_OBJECT;
    node->json_object.next = next;

    return node;
}

json_ast_t *make_object_key_node(parser_t *parser, arena_t *arena)
{
    token_t token = {0};
    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_OBJECT_KEY;

    lexer_next(parser->lexer, &token);

    // Token has to be JSON_STRING_TOKEN (see parse_json_object)
    node->json_object_key.string = make_string_node(&token, arena);

    lexer_next(parser->lexer, &token);

    if (token.json_token != JSON_COLON_TOKEN)
    {
        printf("parse error\n");
        return NULL;
    }

    lexer_next(parser->lexer, &token);

    if (token.json_token == JSON_STRING_TOKEN)
    {
        node->json_object_key.value = make_string_node(&token, arena);
    }

    else if (token.json_token == JSON_LEFT_BRACE_TOKEN)
    {
        node->json_object_key.value = parse_json_object(parser, arena);
    }

    else if (token.json_token == JSON_LEFT_BRACKET_TOKEN)
    {
        node->json_object_key.value = parse_json_array(parser, arena);
    }

    else if (token.json_token == JSON_NUMBER_TOKEN)
    {
        node->json_object_key.value = make_number_node(&token, arena);
    }

    else if (token.json_token == JSON_FALSE_TOKEN || token.json_token == JSON_TRUE_TOKEN)
    {
        node->json_object_key.value = make_boolean_node(&token, arena);
    }

    else if (token.json_token == JSON_NULL_TOKEN)
    {
        node->json_object_key.value = make_null_node(arena);
    }

    else
    {
        printf("parse error\n");
        return NULL;
    }

    return node;
}

void append_object_key_node(parser_t *parser, json_ast_t *head, arena_t *arena)
{
    json_ast_t *new_node = make_object_key_node(parser, arena);
    json_ast_t *current = head;

    while (current->json_object_key.next != NULL)
    {
        current = current->json_object_key.next;
    }

    current->json_object_key.next = new_node;
}

json_ast_t *parse_json_object(parser_t *parser, arena_t *arena)
{
    token_t token = {0};
    json_ast_t *head = NULL;

    for (;;)
    {
        // Token consumed in make_object_key_node
        lexer_peek(parser->lexer, &token);

        if (token.json_token == JSON_STRING_TOKEN)
        {
            if (head != NULL)
            {
                append_object_key_node(parser, head, arena);
            }
            else
            {
                head = make_object_key_node(parser, arena);
            }
        }

        lexer_next(parser->lexer, &token);

        // Object is comma separated list of key values
        if (token.json_token != JSON_COMMA_TOKEN)
        {
            break;
        }
    }

    if (token.json_token != JSON_RIGHT_BRACE_TOKEN)
    {
        printf("parse error\n");
        return NULL;
    }

    return make_object_node(head, arena);
}

json_ast_t *parse_json_array(parser_t *parser, arena_t *arena)
{
    // TODO: Implement array parsing

    return NULL;
}

json_ast_t *parse_json(parser_t *parser, arena_t *arena)
{
    token_t token = {0};
    json_ast_t *json_ast = NULL;

    lexer_next(parser->lexer, &token);

    if (token.json_token == JSON_LEFT_BRACE_TOKEN)
    {
        json_ast = parse_json_object(parser, arena);
    }

    return json_ast; // Returns either built AST or null
}

int main(int argc, char *argv[])
{
    arena_t *arena = arena_init(MiB);
    const char *path = argv[1];

    if (argc != 2)
    {
        printf("Usage: jparse <file>\n"
               "Parse JSON provided by <file>.\n");

        return 1;
    }

    reader_t reader = reader_init(path);
    lexer_t lexer = lexer_init(&reader, arena);

    parser_t parser = {0};

    parser.lexer = &lexer;
    json_ast_t *json = parse_json(&parser, arena);

    return 0;
}
