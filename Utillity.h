#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <typeinfo>
#include "constants.h"
#include "DataTypes.h"

#ifndef UTILLITY_DUPLICATE_DETECTION_H
#define UTILLITY_DUPLICATE_DETECTION_H

/*extern "C" char* find_and_clean(char* p, char target)
{
    while (*p && *p != target)
    { 
         printf("%p -- %c : %c \n",p,*p,lut[*p]);
         *p = lut[(unsigned char)*p]; ++p;
    }
    return (*p == target) ? p : nullptr;
}

extern "C" char* find_and_clean_short(char* p, unsigned short target)
{
    while (*p && (((unsigned char)p[0] << 8) | (unsigned char)p[1]) != target) 
    {
        //printf("%p -- %c : %c  \n", p, *p, lut[(unsigned char)*p]);
        *p = lut[(unsigned char)*p];
        ++p;
    }
    return (((unsigned char)p[0] << 8) | (unsigned char)p[1]) == target ? p : nullptr;
}*/

inline void print_tuple(const uintptr_t* data, const char* format) {
    int field = 0;
    for (size_t i = 0; format[i]; ++i) {
        if (format[i] == '%') {
            ++i;
            if (!format[i]) break;
            char type = format[i];
            if (type == 'V') type = 's'; // V wie s behandeln

            switch (type) {
                case 's': {
                    const char* str = (const char*)data[field];
                    printf("%s", (str && str[0] != '\0') ? str : "<leer>");
                    ++field;
                    break;
                }
                case 'd': {
                    int val = (int)data[field];
                    printf("%d", val);
                    ++field;
                    break;
                }
                case 'f': {
                    double d;
                    memcpy(&d, &data[field], sizeof(double));
                    printf("%f", d);
                    ++field;
                    break;
                }
                case '_':
                    // skip field, do not print, do NOT increment field!
                    break;
                default:
                    // unknown, print as pointer
                    printf("%p", (void*)data[field]);
                    ++field;
            }
            // Optional: Trennzeichen zwischen Feldern
            if (format[i+1] && format[i+1] != '\0' && format[i+1] != '\n')
                printf(" | ");
        }
    }
    printf("\n");
}


template<unsigned int N>
void print_row(const tuple_t<N, uintptr_t>& row, const char* format) {
    print_tuple(row.data, format);
}

template<typename t>
inline void print_Dataset(const dataSet<t>& dataset, const char* format) 
{
    printf("Dataset size: %zu\n", dataset.size);
    for (size_t i = 0; i < dataset.size; ++i)
    {
        print_row<(unsigned int)(sizeof(t)/sizeof(uintptr_t))>(dataset.data[i], format);
    }
}


inline char* find_and_clean_csv(char* p) {
    char* start = p;
    //fprintf(stderr, "[csv] Feldstart (\") @ %p\n", (void*)p);

    if (*p == '"') {
        // Quoted field
        char* dst = p;
        ++p;  // Skip leading quote
        //fprintf(stderr, "[csv] Start quoted @ %p\n", (void*)p);

        while (*p) 
        {
            if (*p == '"') 
            {
                if (*(p + 1) == '"') 
                {
                    // Escaped quote ("")
                    *dst++ = '"';
                    //fprintf(stderr, "[csv] Escaped quote \"\" gefunden @ %p\n", (void*)p);
                    p += 2;
                } else {
                    // End of quoted field
                    ++p;
                    //fprintf(stderr, "[csv] Ende quoted @ %p\n", (void*)p);
                    break;
                }
            } else {
                *dst++ = lut[(unsigned char)*p];
                //fprintf(stderr, "[csv] Quoted-Zeichen: '%c' → '%c' @ %p\n", *p, *(dst - 1), (void*)p);
                ++p;
            }
        }

        // Skip trailing whitespace or unexpected characters until comma or newline
        while (*p && *p != ',' && *p != '\n' && *p != '\r') {
            //fprintf(stderr, "[csv] Überspringe char nach quoted Feld: '%c' @ %p\n", *p, (void*)p);
            ++p;
        }

        //fprintf(stderr, "[csv] Feldende (\") %p -- %zu\n", (void*)p, (size_t)(p-start));
        return p;
    } else {
        // Unquoted field
        char* dst = p;
        while (*p && *p != ',' && *p != '\n' && *p != '\r') {
            *dst++ = lut[(unsigned char)*p];
            //fprintf(stderr, "[csv] Plain-Zeichen: '%c' → '%c' @ %p\n", *p, *(dst - 1), (void*)p);
            ++p;
        }

        //fprintf(stderr, "[csv] Feldende (plain) bei: '%c' @ %p\n", *p, (void*)p);
        return p;
    }
}



// Zählt belegende Platzhalter im Formatstring (ohne %_)
inline int count_fields(const char* format) {
    int count = 0;
    for (size_t i = 0; format[i]; ++i) {
        if (format[i] == '%') {
            ++i;
            if (!format[i]) break;
            switch (format[i]) {
                case 's':
                case 'f':
                case 'd':
                case 'V':
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

// Prüft, ob Structgröße zur Feldanzahl passt
template<typename T>
inline void check_struct_size(const char* format) {
    int n_fields = count_fields(format);
    size_t expected = n_fields * sizeof(uintptr_t);
    size_t actual = sizeof(T);
    if (expected != actual) {
        printf("WARNUNG: Struct %s hat %zu Bytes, erwartet werden %d * %zu = %zu Bytes!\n",
               typeid(T).name(), actual, n_fields, sizeof(uintptr_t), expected);
    }
}

#endif