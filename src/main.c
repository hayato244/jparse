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

json_ast_t *make_value_node(parser_t *parser, arena_t *arena)
{
    token_t token = {0};
    lexer_next(parser->lexer, &token);

    if (token.json_token == JSON_STRING_TOKEN)
    {
        return make_string_node(&token, arena);
    }

    if (token.json_token == JSON_LEFT_BRACE_TOKEN)
    {
        return parse_json_object(parser, arena);
    }

    if (token.json_token == JSON_LEFT_BRACKET_TOKEN)
    {
        return parse_json_array(parser, arena);
    }

    if (token.json_token == JSON_NUMBER_TOKEN)
    {
        json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

        node->tag = JSON_NUMBER;

        // TODO: Implement own number parsing function
        node->number = strtof(token.value, NULL);

        return node;
    }

    if (token.json_token == JSON_FALSE_TOKEN || token.json_token == JSON_TRUE_TOKEN)
    {
        json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

        node->tag = JSON_BOOLEAN;
        node->boolean = token.json_token == JSON_TRUE_TOKEN ? true : false;

        return node;
    }

    if (token.json_token == JSON_NULL_TOKEN)
    {
        json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

        node->tag = JSON_NULL;
        node->null = NULL;

        return node;
    }

    return NULL;
}

json_ast_t *parse_json_object(parser_t *parser, arena_t *arena)
{
    token_t token = {0};
    json_ast_t *head = NULL;

    for (;;)
    {
        lexer_next(parser->lexer, &token);

        if (token.json_token == JSON_RIGHT_BRACE_TOKEN)
        {
            break;
        }
        else if (token.json_token == JSON_STRING_TOKEN)
        {
            // Build object key value node
            json_ast_t *new_node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

            new_node->tag = JSON_OBJECT_KEY;
            new_node->json_object_key.string = make_string_node(&token, arena);

            lexer_next(parser->lexer, &token);

            if (token.json_token != JSON_COLON_TOKEN)
            {
                return NULL;
            }

            new_node->json_object_key.value = make_value_node(parser, arena);

            if (new_node->json_object_key.value == NULL)
            {
                return NULL;
            }

            if (head != NULL)
            {
                json_ast_t *current = head;

                while (current->json_object_key.next != NULL)
                {
                    current = current->json_object_key.next;
                }

                current->json_object_key.next = new_node;
            }
            else
            {
                head = new_node;
            }
        }
        else
        {
            return NULL;
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
        return NULL;
    }

    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_OBJECT;
    node->json_object.next = head;

    return node;
}

json_ast_t *parse_json_array(parser_t *parser, arena_t *arena)
{
    token_t token = {0};
    json_ast_t *head = NULL;

    for (;;)
    {
        // Use peek to prevent consuming token used for value node
        lexer_peek(parser->lexer, &token);

        if (token.json_token == JSON_RIGHT_BRACKET_TOKEN)
        {
            lexer_next(parser->lexer, &token);
            break;
        }
        else if (head != NULL)
        {
            json_ast_t *new_node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);
            json_ast_t *current = head;

            while (current->json_array_value.next != NULL)
            {
                current = current->json_array_value.next;
            }

            new_node->json_array_value.value = make_value_node(parser, arena);
            current->json_array_value.next = new_node;
        }
        else
        {
            head = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);
            head->json_array_value.value = make_value_node(parser, arena);
        }

        lexer_next(parser->lexer, &token);

        // Array is comma separated list of values
        if (token.json_token != JSON_COMMA_TOKEN)
        {
            break;
        }
    }

    if (token.json_token != JSON_RIGHT_BRACKET_TOKEN)
    {
        return NULL;
    }

    json_ast_t *node = (json_ast_t *)ARENA_ALLOC_STRUCT(arena, json_ast_t);

    node->tag = JSON_ARRAY;
    node->json_array.next = head;

    return node;
}

json_ast_t *parse_json(parser_t *parser, arena_t *arena)
{
    token_t token = {0};
    json_ast_t *json_ast = NULL;

    lexer_peek(parser->lexer, &token);

    switch (token.json_token)
    {
        case JSON_LEFT_BRACE_TOKEN:
            lexer_next(parser->lexer, &token);
            json_ast = parse_json_object(parser, arena);
            break;

        case JSON_LEFT_BRACKET_TOKEN:
            lexer_next(parser->lexer, &token);
            json_ast = parse_json_array(parser, arena);
            break;

        case JSON_RIGHT_BRACE_TOKEN:
        case JSON_RIGHT_BRACKET_TOKEN:
        case JSON_COLON_TOKEN:
        case JSON_COMMA_TOKEN:
        case JSON_NONE_TOKEN:
            break;

        default:
            json_ast = make_value_node(parser, arena);
            break;
    }

    lexer_next(parser->lexer, &token);

    if (json_ast != NULL && token.type != EOF_TOKEN)
    {
        return NULL;
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
    parse_json(&parser, arena);

    return 0;
}
