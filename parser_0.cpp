#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include "constants.h"
extern "C" char* find_and_clean(char* p, char target)
{
   while (*p && *p != target)
   {
       printf("%p -- %c : %c \n",p,*p,lut[*p]);
       *p = lut[(unsigned char)*p]; ++p;
   }
   return (*p == target) ? p : nullptr;
}

extern "C" size_t parser_0(char* line, void* out) {
    char* p = line;
    char* end = nullptr;
    uintptr_t* fields = (uintptr_t*)out;
    // Skip field
    if (*p == '"') {
        p = strchr(p + 1, '"');
        if (p) p = strchr(p + 1, ',');
    } else {
        p = strchr(p, ',');
    }
    if (p) ++p;
    if (*p == '"') {
        ++p;
        end = find_and_clean(p, '"');
        if (end) *end = '\0';
        fields[0] = (uintptr_t)p;
        p = end + 1;
        if (*p == ',') ++p;
    } else {
        end = find_and_clean(p, ',');
        if (!end) end = p + strlen(p);
        fields[0] = (uintptr_t)p;
        if (*end != '\0') *end = '\0';
        p = (*end == ',') ? end + 1 : end;
    }
    return p - line;
}
