#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include "constants.h"
extern "C" char* find_and_clean(char* p, char target)
{
   while (*p && *p != target) { *p = lut[(unsigned char)*p]; ++p; }
   return (*p == target) ? p : nullptr;
}

extern "C" size_t parser_2(char* line, void* out) {
    char* p = line;
    char* end = nullptr;
    uintptr_t* fields = (uintptr_t*)out;
    fields[0] = (uintptr_t)atoi(p);
    p = strchr(p, ',');
    if (p) ++p;
    fields[1] = (uintptr_t)atoi(p);
    p = strchr(p, ',');
    if (p) ++p;
    return p - line;
}
