#ifndef MATCHING_MANAGER
#define MATCHING_MANAGER

#include <stdlib.h>
#include "DataTypes.h"

template<typename T>
class Matching_mngr {
public:
    Matching_mngr() {}
    ~Matching_mngr() {}

    // Vergleicht alle Einträge innerhalb eines Blocks im Bereich [start, start+span)
    // Nutzt den == Operator des Typs T für den Vergleich
    matching match_block(
        const partition& part,
        unsigned int start,
        unsigned int span
    ) {
        unsigned int end = start + span;
        if (end > part.size) end = part.size;

        match* temp_matches = (match*)malloc((end - start) * part.size * sizeof(match));
        unsigned int match_count = 0;

        for (unsigned int i = start; i < end; ++i) {
            const T* entry_i = reinterpret_cast<const T*>(part.data[i].data[1]);
            uintptr_t id_i = part.data[i].data[0];
            for (unsigned int j = i + 1; j < part.size; ++j) {
                const T* entry_j = reinterpret_cast<const T*>(part.data[j].data[1]);
                uintptr_t id_j = part.data[j].data[0];
                // --- Vergleich mit == Operator ---
                if (*entry_i == *entry_j) {
                    temp_matches[match_count].data[0] = id_i;
                    temp_matches[match_count].data[1] = id_j;
                    match_count++;
                }
            }
        }

        matching result;
        result.matches = (match*)malloc(match_count * sizeof(match));
        memcpy(result.matches, temp_matches, match_count * sizeof(match));
        result.size = match_count;
        free(temp_matches);
        return result;
    }
};

#endif