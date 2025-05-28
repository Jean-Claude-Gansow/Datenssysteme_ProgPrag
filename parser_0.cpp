#include <cstring>
extern "C" void parser_0(char* line, void** out) {
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
        end = strchr(p, '"');
        if (end) *end = '\0';
        ((char**)out[0])[0] = p;
        p = end + 1;
        if (*p == ',') ++p;
    }
	else
	{
        end = strchr(p, ',');
        if (!end) end = p + strlen(p);
        ((char**)out[0])[0] = p;
        if (*end != '\0') *end = '\0';
        p = (*end == ',') ? end + 1 : end;
    }
}
