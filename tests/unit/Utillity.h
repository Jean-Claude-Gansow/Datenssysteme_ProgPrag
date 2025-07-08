#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdint>
#include <typeinfo>
#include <fstream>
#include <sstream>
#include "constants.h"
#include "DataTypes.h"

#ifndef UTILLITY_DUPLICATE_DETECTION_H
#define UTILLITY_DUPLICATE_DETECTION_H

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

// Gibt alle Partitionen sortiert aus, jeweils die Zahl des gewünschten Feldes (z.B. ID = 0, Pointer = 1)
inline void print_partitions_field(const dataSet<partition> &partitions, size_t field_idx)
{
    for (size_t i = 0; i < partitions.size; ++i)
    {
        printf("Partition %zu: ", i);
        for (size_t j = 0; j < partitions.data[i].size; ++j)
        {
            printf("%zu", (size_t)partitions.data[i].data[j].data[field_idx]);
            if (j + 1 < partitions.data[i].size)
                printf(", ");
        }
        printf("\n");
    }
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
    printf("done printing\n");
}

inline std::string read_file(const std::string &filename)
{
    std::ifstream in(filename);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

inline char* find_and_clean_csv(char* p) {
    char* start = p;
    
    if (*p == '"') {
        // Quoted field
        char* dst = p;  // Start writing at beginning of field
        ++p;  // Skip leading quote
        
        // Process the quoted content
        while (*p) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    // Properly handle escaped quotes ("") by converting to Escape character
                    *dst++ = Escape;
                    p += 2;  // Skip both quote characters
                } else {
                    // End of quoted field
                    *dst = '\0';  // Properly null-terminate the output string
                    ++p;  // Move past the closing quote
                    break;
                }
            } else {
                // Process normal character using the lookup table
                *dst++ = lut[(unsigned char)*p];
                ++p;
            }
        }
        
        // Skip trailing whitespace or unexpected characters until delimiter
        while (*p && *p != ',' && *p != '\n' && *p != '\r') {
            ++p;
        }
        
        return p;
    } else {
        // Unquoted field
        char* dst = p;
        
        // Process all characters until delimiter
        while (*p && *p != ',' && *p != '\n' && *p != '\r') {
            // In-place transformation using lookup table
            *dst++ = lut[(unsigned char)*p];
            ++p;
        }
        
        // Ensure proper null-termination if content was modified
        if (dst < p) {
            *dst = '\0';
        }
        
        return p;
    }
}

inline void replace_all(std::string &target, const std::string &placeholder, const std::string &value)
{
    size_t pos;
    while ((pos = target.find(placeholder)) != std::string::npos)
    {
        target.replace(pos, placeholder.length(), value);
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