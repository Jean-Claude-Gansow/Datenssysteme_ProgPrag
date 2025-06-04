#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include "constants.h"
#include "Utillity.h"

// --- Abschnitt für %s (String-Feld) ---
inline void parse_field_s(char*& p, uintptr_t* fields, int idx, char* line, FILE* outbuffer)
{
    fprintf(outbuffer, "[parser] s Feldstart %d: '%c' @ %ld\n", idx, *p, p - line);
    if (*p == ',' || *p == '\n' || *p == '\0' || *p == '\r') {
        fields[idx] = (uintptr_t)"";
        if (*p == ',') ++p;
    } else {
        char* end = find_and_clean_csv(p);
        fields[idx] = (uintptr_t)p;
        char trenn = *end;
        if (trenn != '\0') *end = '\0';
        p = end;
        if (trenn == ',') ++p;
    }
    fprintf(outbuffer, "[parser] Feldende %d: '%c' @ %ld\n", idx, *p, p - line);
}

// --- Abschnitt für %f (Float-Feld) ---
inline void parse_field_f(char*& p, uintptr_t* fields, int idx, char* line, FILE* outbuffer) {
    fprintf(outbuffer, "[parser] f Feldstart %d: '%c' @ %ld\n", idx, *p, p - line);
    if (*p == ',' || *p == '\n' || *p == '\0' || *p == '\r') {
        float tmp_f = 0.0f;
        uintptr_t tmp_bits;
        memcpy(&tmp_bits, &tmp_f, sizeof(float));
        fields[idx] = tmp_bits;
        if (*p == ',') ++p;
    } else {
        char* end;
        float tmp_f = strtof(p, &end);
        uintptr_t tmp_bits;
        memcpy(&tmp_bits, &tmp_f, sizeof(float));
        fields[idx] = tmp_bits;
        p = end;
        if (*p == ',') ++p;
    }
    fprintf(outbuffer, "[parser] Feldende %d: '%c' @ %ld\n", idx, *p, p - line);
}

// --- Abschnitt für %_ (ignore) ---
inline void parse_field_ignore(char*& p, char* line, FILE* outbuffer) {
    fprintf(outbuffer, "[parser] ign. Feldstart: '%c' @ %ld\n", *p, p - line);
    if (*p == '\"') {
        p = strchr(p + 1, '\"');
        if (p) p = strchr(p + 1, ',');
    } else {
        p = strchr(p, ',');
    }
    if (p) ++p;
    fprintf(outbuffer, "[parser] ign. Feldende: '%c' @ %ld\n", *p, p - line);
}

// --- Abschnitt für %V (Rest der Zeile als String) ---
inline void parse_field_V(char*& p, uintptr_t* fields, int idx, char* line, FILE* outbuffer) {
    fprintf(outbuffer, "[parser] V Feldstart %d: '%c' @ %ld\n", idx, *p, p - line);
    if (*p == '\n' || *p == '\0' || *p == '\r') {
        fields[idx] = (uintptr_t)"";
    } else {
        char* end = find_and_clean_csv(p);
        fprintf(outbuffer, "found End at: %c\n", *end);
        fields[idx] = (uintptr_t)p;
        if (*end != '\0') *end = '\0';
        p = end;
    }
    fprintf(outbuffer, "[parser] Feldende %d: '%c' @ %ld\n", idx, *p, p - line);
}

// --- Hauptfunktion ---
extern "C" size_t {{FUNC_NAME}}(char* line, void* out) {
    FILE* outbuffer = fopen("{{FUNC_NAME}}_out.txt", "a");
    if (!outbuffer) outbuffer = stdout;
    char* p = line;
    uintptr_t* fields = (uintptr_t*)out;
{{FORMAT_CODE}}
    if (outbuffer && outbuffer != stdout) fclose(outbuffer);
    return p - line;
}