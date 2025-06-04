#include <stdio.h>
#include <cstdio>
#include <cstring>
#include "constants.h"
#include "DataTypes.h"

extern "C" char* find_and_clean(char* p, char target)
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
}

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
                    break;
                }
                case 'd': {
                    int val = (int)data[field];
                    printf("%d", val);
                    break;
                }
                case 'f': {
                    float f;
                    memcpy(&f, &data[field], sizeof(float));
                    printf("%f", f);
                    break;
                }
                case '_':
                    // skip field, do not print
                    break;
                default:
                    // unknown, print as pointer
                    printf("%p", (void*)data[field]);
            }
            ++field;
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
inline void print_row(char* row, const char*) {
    printf("%s\n", row ? row : "");
}
inline void print_row(const match& row, const char*) {
    printf("%d, %d\n", row.data[0], row.data[1]);
}

extern "C" char* find_and_clean_csv(char* p) {
    while (*p) {
        if (*p == '"') {
            ++p;//start infield reading
            // Replace all characters until the next quote or end of string
            printf("found quote at %p\n", p);
            while (*p && *p != '"') 
                {
                    printf("changing -- %c : %c\n", *p, lut[*p]); 
                    *p = lut[*p]; 
                    ++p;
                }// replace characters
            if (*p == '"') ++p;
        }
        if (*p == ',' || *p == '\n' || *p == '\r' || *p == '\0') {
            return p;
        }
        ++p;
    }
    return p;
}