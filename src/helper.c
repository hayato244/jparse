#include <stdbool.h>

#include "helper.h"

bool is_whitespace(char c)
{
    return (c >= '\t' && c <= '\r') || (c == ' ');
}

bool is_numeric(char c)
{
    return (c >= '0' && c <= '9') || c == '-' || c == '+' || c == 'e' || c == 'E' || c == '.';
}

bool is_alpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}