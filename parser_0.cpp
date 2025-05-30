#include <cstring>
#include "constants.h"
char* find_and_clean(char* p, char target)
{
   while (*p && *p != target)
	{
       *p = lut[(unsigned char)*p];
       ++p;
   }
   return (*p == target) ? p : nullptr;
}

extern "C" size_t parser_0(char* line, void** out) {
    char* p = line;
    char* end = nullptr;
    // Skip field
    if (*p == '"')
	{
        p = strchr(p + 1, '"');
        if (p) p = strchr(p + 1, ',');
    }
	else
	{
        p = strchr(p, ',');
    }
    if (p) ++p;
    if (*p == '"')
	{
        ++p;
        end = find_and_clean(p, '"');
        if (end) *end = '\0';
        ((char**)out[0])[0] = p;
        p = end + 1;
        if (*p == ',') ++p;
    }
	else
	{
        end = find_and_clean(p, ',');
        if (!end) end = p + strlen(p);
        ((char**)out[0])[0] = p;
        if (*end != '\0') *end = '\0';
        p = (*end == ',') ? end + 1 : end;
    }
return p - line;
}
