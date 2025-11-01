#pragma once

#include <stdbool.h>
#include <string.h>

bool string_equals(const char * a, const char * b) { return strcmp(a, b) == 0; }

bool starts_with(const char * prefix, const char * str) { return strncmp(prefix, str, strlen(prefix)) == 0; }