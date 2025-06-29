#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include "constants.h"
#include "Utillity.h"

#ifndef COPY_STRING_FIELDS
#define COPY_STRING_FIELDS 1  // 0 = Pointer merken, 1 = strdup
#endif

// #define PRINT_FILE_OUTPUT 1  // definiert -> Ausgabe in Datei 

// --- Abschnitt für %s (String-Feld) ---
inline void parse_field_s(char*& p, uintptr_t* fields, int idx, char* line)
{
    if (*p == ',' || *p == '\n' || *p == '\0' || *p == '\r') {
        fields[idx] = (uintptr_t)(COPY_STRING_FIELDS ? strdup("") : "");
        if (*p == ',') ++p;
    } else {
        char* end = find_and_clean_csv(p);
        char trenn = *end;
        if (trenn != '\0') *end = '\0';

        fields[idx] = (uintptr_t)(
            COPY_STRING_FIELDS ? strdup(p) : p
        );

        p = end;
        if (trenn == ',') ++p;
    }
}


// --- Abschnitt für %f (Double-Feld) ---
inline void parse_field_f(char*& p, uintptr_t* fields, int idx, char* line) {
    if (*p == ',' || *p == '\n' || *p == '\0' || *p == '\r') {
        // Leeres Feld => 0.0
        double zero = 0.0;
        uintptr_t bits;
        memcpy(&bits, &zero, sizeof(zero));
        fields[idx] = bits;
        if (*p == ',') ++p;
        return;
    }

    // Float-Wert parsen
    char* end;
    double val = strtod(p, &end);
    uintptr_t bits;
    memcpy(&bits, &val, sizeof(val));
    fields[idx] = bits;
    p = end;
    if (*p == ',') ++p;
}


// --- Abschnitt für %d (Integer-Feld) ---
inline void parse_field_d(char*& p, uintptr_t* fields, int idx, char* line)
{
    // Überspringe ggf. Leerzeichen
    if (*p == ',' || *p == '\n' || *p == '\0' || *p == '\r') {
        fields[idx] = 0;
        if (*p == ',') ++p;
    } else {
        char* endptr;
        long value = strtol(p, &endptr, 10);  // liest Integer, schreibt neue Position nach endptr

        fields[idx] = (uintptr_t)value;
        //fprintf(outbuffer,"[%p]: found %ld at %ld\n",p,value, p - line);
        p = endptr; // weiter nach Zahl

        if (*p == ',') ++p;
    }
}

// --- Abschnitt für %_ (ignore) ---
inline void parse_field_ignore(char*& p, char* line) 
{
    if (*p == '"') {
        ++p;  // Startquote überspringen
        while (*p) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2;  // Escaped quote ("" → ")
                } else {
                    ++p;  // Endquote
                    break;
                }
            } else {
                ++p;
            }
        }
    }

    // Nach Feldende: , oder Zeilenende überspringen
    while (*p && *p != ',' && *p != '\n' && *p != '\r') ++p;
    if (*p == ',') ++p;
}


// --- Abschnitt für %V (Rest der Zeile als String) ---
inline void parse_field_V(char*& p, uintptr_t* fields, int idx, char* line) 
{
    if (*p == '\n' || *p == '\0' || *p == '\r') {
        fields[idx] = (uintptr_t)(COPY_STRING_FIELDS ? strdup("") : (char*)"");
        //fprintf(outbuffer, "[parser] Leeres V-Feld erkannt\n");
        return;
    }

    char* start = p;
    char* end = find_and_clean_csv(p);

    // Nullterminieren, damit wir ein valides C-Stringende für das Feld haben
    if (*end != '\0') {
        *end = '\0';
    }

    fields[idx] = (uintptr_t)(
        COPY_STRING_FIELDS ? strdup(start) : start
    );
    p = end + 1;  // weiter zum nächsten Feld oder '\0'
}



// --- Hauptfunktion ---
extern "C" size_t parser_1(char* line, void* out) 
{
    char* p = line;
    uintptr_t* fields = (uintptr_t*)out;
    parse_field_ignore(p, line);
    parse_field_s(p, fields, 0, line);
    parse_field_ignore(p, line);
    parse_field_s(p, fields, 1, line);
    parse_field_s(p, fields, 2, line);
    parse_field_V(p, fields, 3, line);

    while (*p == '\r' || *p == '\n')
    {++p;}
return p - line;
}