#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include "constants.h"
#include "Utillity.h"
extern "C" size_t parser_1(char* line, void* out) {
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
    printf("[parser] Feldstart %d: '%c' @ %ld\n", 0, *p, p-line);
    if (*p == ',') {
        fields[0] = (uintptr_t)"";
        ++p;
    } else if (*p == '"') {
        ++p;
        end = find_and_clean_short(p, 8748 );
        if (end) *end = '\0';
        fields[0] = (uintptr_t)p;
        p = end ? end + 1 : p + strlen(p);
        if (*p == ',') ++p;
    } else {
        end = find_and_clean(p, ',');
        if (!end) end = p + strlen(p);
        fields[0] = (uintptr_t)p;
        if (*end != '\0') *end = '\0';
        p = (*end == ',') ? end + 1 : end;
    }
    printf("[parser] Feldende %d: '%c' @ %ld\n", 0, *p, p-line);
    if (*p == ',' || *p == '\n' || *p == '\0') {
        float tmp_f = 0.0f;
        uintptr_t tmp_bits;
        memcpy(&tmp_bits, &tmp_f, sizeof(float));
        fields[1] = tmp_bits;
        printf("[parser] float field leer, setze 0.0 an pos %d\n", 1);
        if (*p == ',') ++p;
    } else {
        float tmp_f = strtof(p, &end);
        printf("[parser] float gelesen: %f aus '%.*s' an pos %d\n", tmp_f, (int)(end-p), p, 1);
        uintptr_t tmp_bits;
        memcpy(&tmp_bits, &tmp_f, sizeof(float));
        fields[1] = tmp_bits;
        p = (*end == ',') ? end + 1 : end;
    }
    printf("[parser] Feldstart %d: '%c' @ %ld\n", 2, *p, p-line);
    if (*p == ',') {
        fields[2] = (uintptr_t)"";
        ++p;
    } else if (*p == '"') {
        ++p;
        end = find_and_clean_short(p, 8748 );
        if (end) *end = '\0';
        fields[2] = (uintptr_t)p;
        p = end ? end + 1 : p + strlen(p);
        if (*p == ',') ++p;
    } else {
        end = find_and_clean(p, ',');
        if (!end) end = p + strlen(p);
        fields[2] = (uintptr_t)p;
        if (*end != '\0') *end = '\0';
        p = (*end == ',') ? end + 1 : end;
    }
    printf("[parser] Feldende %d: '%c' @ %ld\n", 2, *p, p-line);
    printf("[parser] Feldstart %d: '%c' @ %ld\n", 3, *p, p-line);
    if (*p == ',') {
        fields[3] = (uintptr_t)"";
        ++p;
    } else if (*p == '"') {
        ++p;
        end = find_and_clean_short(p, 8748 );
        if (end) *end = '\0';
        fields[3] = (uintptr_t)p;
        p = end ? end + 1 : p + strlen(p);
        if (*p == ',') ++p;
    } else {
        end = find_and_clean(p, ',');
        if (!end) end = p + strlen(p);
        fields[3] = (uintptr_t)p;
        if (*end != '\0') *end = '\0';
        p = (*end == ',') ? end + 1 : end;
    }
    printf("[parser] Feldende %d: '%c' @ %ld\n", 3, *p, p-line);
    if (*p == ',') {
        fields[4] = (uintptr_t)"";
        ++p;
    } else if (*p == '"') {
        ++p;
        end = find_and_clean_short(p, 8714 );
        if (end) *end = '\0';
        fields[4] = (uintptr_t)p;
        p = end ? end + 1 : p + strlen(p);
    } else {
        end = find_and_clean(p, '\n');
        if (!end) end = p + strlen(p);
        fields[4] = (uintptr_t)p;
        if (*end != '\0') *end = '\0';
        p = end;
        if (*p == '\n') ++p;
    }
    return p - line;
}
