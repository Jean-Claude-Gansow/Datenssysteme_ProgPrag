#ifndef MATCHING_MANAGER
#define MATCHING_MANAGER

#include <stdlib.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <cmath>
#include "DataTypes.h"
#include <unordered_set>

template<typename compType>
class Matching_mngr {
private:
    // Keine match_counts mehr notwendig
    std::mutex match_mutex; // Für Thread-Sicherheit bei Bedarf

    // Statisches Array für die Sets, die für den Jaccard-Vergleich verwendet werden
    std::unordered_set<uint32_t>** jaccard_cache;
    size_t unique_id_count;

public:
    Matching_mngr(size_t unique_id_count) : unique_id_count(unique_id_count) 
    {    
        printf("initializing match cache for %zu uinique IDs\n",unique_id_count);
        jaccard_cache = new std::unordered_set<uint32_t>*[unique_id_count];
        // Alle Einträge auf nullptr setzen
        for (size_t i = 0; i < unique_id_count; ++i) {
            jaccard_cache[i] = nullptr;
        }
    }

    ~Matching_mngr() {
        // Speicher der Sets freigeben
        if (jaccard_cache != nullptr) {
            for (size_t i = 0; i < unique_id_count; ++i) {
                if (jaccard_cache[i] != nullptr) {
                    delete jaccard_cache[i];
                }
            }
            delete[] jaccard_cache;
        }
    }

    // Wrapper-Funktion für den Jaccard-Vergleich
    bool jaccard_compare(uintptr_t id_i, compType* entry_i, uintptr_t id_j, compType* entry_j) {
        // Überprüfen, ob die Einträge gültig sind
        if (entry_i == nullptr || entry_j == nullptr) {
            return false;
        }

        // Überprüfen, ob die IDs innerhalb des Bereichs liegen
        if (id_i >= unique_id_count || id_j >= unique_id_count) {
            fprintf(stderr, "Error: IDs out of range (%lu, %lu)\n", id_i, id_j);
            return false;
        }

        // Überprüfen, ob die Sets bereits im Cache vorhanden sind
        // Die bereits existierenden Buffer aus match_blocker_intern werden verwendet
        if (jaccard_cache[id_i] == nullptr && entry_i->numeral_buffer != nullptr) {
            jaccard_cache[id_i] = entry_i->generate_set(); // generate_set erstellt das Set
        }

        if (jaccard_cache[id_j] == nullptr && entry_j->numeral_buffer != nullptr) {
            jaccard_cache[id_j] = entry_j->generate_set(); // generate_set erstellt das Set
        }

        // Prüfen, ob die Sets erfolgreich erstellt wurden
        if (jaccard_cache[id_i] == nullptr || jaccard_cache[id_j] == nullptr) {
            return false;
        }

        // Jaccard-Vergleich durchführen
        const std::unordered_set<uint32_t>& set_i = *(jaccard_cache[id_i]);
        const std::unordered_set<uint32_t>& set_j = *(jaccard_cache[id_j]);

        // Vermeiden von Division durch Null
        if (set_i.empty() && set_j.empty()) {
            return false;
        }

        size_t intersection_size = 0;
        for (const auto& elem : set_i) {
            if (set_j.find(elem) != set_j.end()) {
                intersection_size++;
            }
        }

        size_t union_size = set_i.size() + set_j.size() - intersection_size;
        double jaccard_index = static_cast<double>(intersection_size) / union_size;

        return jaccard_index >= 0.93;
    }

    void match_blocker_intern(
        partition* part, 
        size_t start, 
        size_t end, 
        matching* findingsBuffer)
    {
        // Initialisiere das findingsBuffer-Objekt sofort
        if (findingsBuffer) {
            findingsBuffer->size = 0;
            findingsBuffer->matches = nullptr;
        }
        
        // Überprüfung der Parameter
        if (part == nullptr || part->data == nullptr || findingsBuffer == nullptr || start >= end) {
            fprintf(stderr, "Error: Invalid parameters in match_blocker_intern\n");
            return;
        }

        // Statische Buffer für numerische Verarbeitung - explizit initialisiert
        uint32_t numeral_buffer1[500];
        uint32_t numeral_buffer2[500];

        // Explizite Initialisierung aller Elemente auf 0
        memset(numeral_buffer1, 0, sizeof(numeral_buffer1));
        memset(numeral_buffer2, 0, sizeof(numeral_buffer2));

        // Maximale Kapazität des Buffers berechnen
        size_t range_size = end - start;
        if (range_size == 0) {
            return;  // Nichts zu tun
        }
        
        size_t max_possible_matches = (range_size * (range_size - 1)) / 2;
        if (max_possible_matches == 0) {
            return;  // Keine möglichen Matches
        }
        
        // Temp-Puffer für die Matches erstellen
        try {
            findingsBuffer->matches = new match[max_possible_matches]();
        } catch (const std::bad_alloc& e) {
            fprintf(stderr, "Error: Memory allocation failed in match_blocker_intern\n");
            return;
        }
        
        size_t match_count = 0;
        
        // Debug-Zähler
        size_t comparison_count = 0;
        
        // Hauptschleife für Vergleiche - nur | Operator
        for (size_t i = start; i < end; i++) 
        {            
            uintptr_t id_i = (uintptr_t)part->data[i][0];
            compType* entry_i = reinterpret_cast<compType*>(part->data[i][1]);
            
            // Vergleiche mit allen nachfolgenden Einträgen
            for (size_t j = i + 1; j < end; ++j) {
                
                uintptr_t id_j = (uintptr_t)part->data[j][0];
                compType* entry_j = reinterpret_cast<compType*>(part->data[j][1]);

                comparison_count++;
                
                // Weise die numeral_buffer zu, bevor jaccard_compare aufgerufen wird
                entry_i->numeral_buffer = numeral_buffer1;
                entry_j->numeral_buffer = numeral_buffer2;

                if (jaccard_compare(id_i, entry_i, id_j, entry_j)) {
                    printf("[%lu == %lu]\n", id_i, id_j);
                    findingsBuffer->matches[match_count].data[0] = id_i;
                    findingsBuffer->matches[match_count].data[1] = id_j;
                    match_count++;
                }
                
            }
        }
        
        // Ergebnisse in den findingsBuffer kopieren
        findingsBuffer->size = match_count;
        // Optional: Speicher auf exakte Größe reduzieren
        if (match_count < max_possible_matches) {
            match* resized = new match[match_count];
            memcpy(resized, findingsBuffer->matches, match_count * sizeof(match));
            delete[] findingsBuffer->matches;
            findingsBuffer->matches = resized;
        }
    }

    dataSet<matching>* identify_matches(dataSet<partition>* input, size_t numThreads = 1)
    {
        // Überprüfen der Eingabe
        if (input == nullptr || input->data == nullptr) {
            fprintf(stderr, "Error: Invalid input in identify_matches\n");
            return nullptr;
        }
        
        size_t num_partitions = input->size;
        matching* matching_buffer = new matching[num_partitions]();
        
        for (size_t p = 0; p < num_partitions; p++) {
            partition* part = &input->data[p];
            
            // Sicherstellen, dass die Partition gültig ist
            if (part == nullptr || part->data == nullptr || part->size <= 1) {
                matching_buffer[p].size = 0;
                matching_buffer[p].matches = nullptr;
                continue;
            }
            
            // Für kleine Partitionen oder wenn nur ein Thread gewünscht ist
            if (part->size < 50 || numThreads == 1) {
                match_blocker_intern(part, 0, part->size, &matching_buffer[p]);
                continue;
            }
            
            // Berechne optimale Partitionierung für Multi-Threading
            size_t* offsets = new size_t[numThreads + 1](); //auf 0 initialisieren
            offsets[0] = 0;
            offsets[numThreads] = part->size;
            
            // Berechne die Thread-Grenzen (Quadratische Verteilung)
            size_t total_comparisons = (part->size * (part->size - 1)) / 2;
            size_t comparisons_per_thread = total_comparisons / numThreads;
            size_t completed_comps = 0;
            
            for (size_t t = 1; t < numThreads; t++) {
                size_t i = offsets[t-1];
                while (i < part->size - 1) {
                    size_t comps_in_row = part->size - i - 1;
                    if (completed_comps + comps_in_row >= comparisons_per_thread * t) {
                        size_t pos_in_row = comparisons_per_thread * t - completed_comps;
                        if (pos_in_row < comps_in_row) {
                            offsets[t] = i + 1;
                            break;
                        }
                    }
                    completed_comps += comps_in_row;
                    i++;
                    offsets[t] = i;
                }
            }
            
            // Erstelle Threads und Ergebnispuffer für jeden Thread
            std::thread** threads = new std::thread*[numThreads]();
            matching* thread_results = new matching[numThreads]();
            
            // Threads starten - einfacherer Code ohne match_counts
            for (size_t t = 0; t < numThreads; t++) {
                size_t start = offsets[t];
                size_t end = offsets[t+1];
                
                if (start < end && end <= part->size) {
                    threads[t] = new std::thread([this, part, start, end, &thread_results, t]() {
                        match_blocker_intern(part, start, end, &thread_results[t]);
                    });
                } else {
                    threads[t] = nullptr;
                }
            }
            
            // Warte auf alle Threads
            for (size_t t = 0; t < numThreads; t++) {
                if (threads[t] && threads[t]->joinable()) {
                    threads[t]->join();
                    delete threads[t];
                }
            }
            delete[] threads;
            
            // Zähle die Gesamtzahl der Matches für die Allokation
            size_t total_matches = 0;
            for (size_t t = 0; t < numThreads; t++) {
                total_matches += thread_results[t].size;
            }
            
            // Ergebnispuffer für die Partition allozieren
            matching_buffer[p].size = total_matches;
            if (total_matches > 0) {
                matching_buffer[p].matches = new match[total_matches]();
                
                // Kopiere alle Matches
                size_t match_idx = 0;
                for (size_t t = 0; t < numThreads; t++) {
                    if (thread_results[t].matches != nullptr) {
                        for (size_t i = 0; i < thread_results[t].size; i++) {
                            matching_buffer[p].matches[match_idx] = thread_results[t].matches[i];
                            match_idx++;
                        }
                    }
                }
                
                // Überprüfe, ob alle Matches kopiert wurden
                if (match_idx != total_matches) {
                    fprintf(stderr, "Error: Match count mismatch after copying (%zu vs %zu)\n",
                            match_idx, total_matches);
                }
            } else {
                matching_buffer[p].matches = nullptr;
            }
            
            // Thread-Ergebnisse freigeben
            for (size_t t = 0; t < numThreads; t++) {
                delete[] thread_results[t].matches;
            }
            delete[] thread_results;
            delete[] offsets;
        }
        
        // Ergebnisse verpacken
        dataSet<matching>* result = new dataSet<matching>();
        result->size = num_partitions;
        result->data = matching_buffer;
        
        return result;
    }
};
#endif