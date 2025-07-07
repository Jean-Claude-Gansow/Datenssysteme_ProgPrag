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
#include "debug_utils.h"

// Konfigurierbare Konstanten
// Maximale Anzahl von Matches, die für ein Element gesucht werden
// Höherer Wert = mehr Matches pro Element werden gefunden, aber langsamere Ausführung
// Niedrigerer Wert = schnellere Ausführung, aber möglicherweise weniger Matches pro Element
#define MAX_MATCHES_PER_ITEM 3

template<typename compType>
class Matching_mngr {
private:
    // Keine match_counts mehr notwendig
    std::mutex match_mutex; // Für Thread-Sicherheit bei Bedarf

public:
    Matching_mngr() {}
    
    ~Matching_mngr() {}
    
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
        
        // Jump-Table für die verschiedenen Ergebnisse
        static void *jumpTable[3] = {
            &&nomatch, &&ismatch, &&fallback
        };
        
        // Maximale Kapazität des Buffers berechnen
        size_t range_size = end - start;
        size_t max_possible_matches = (range_size * (range_size - 1)) / 2;
        
        // Temp-Puffer für die Matches erstellen
        match* matches = new match[max_possible_matches]();
        size_t match_count = 0;
        
        // Debug-Zähler
        size_t comparison_count = 0;
        
        // Hauptschleife für Vergleiche - keine Überprüfung auf MAX_MATCHES_PER_ITEM mehr
        for (size_t i = start; i < end; i++) {
            // Validierung des Eintrags
            if (!part->data[i][1]) {
                fprintf(stderr, "Warning: Entry at index %zu is NULL, skipping\n", i);
                continue;
            }
            
            uintptr_t id_i = (uintptr_t)part->data[i][0];
            const compType* entry_i = reinterpret_cast<compType*>(part->data[i][1]);
            
            // Vergleiche mit allen nachfolgenden Einträgen
            for (size_t j = i + 1; j < end; ++j) {
                // Validierung des zu vergleichenden Eintrags
                if (!part->data[j][1]) {
                    fprintf(stderr, "Warning: Entry at index %zu is NULL, skipping\n", j);
                    continue;
                }
                
                uintptr_t id_j = (uintptr_t)part->data[j][0];
                const compType* entry_j = reinterpret_cast<compType*>(part->data[j][1]);
                
                comparison_count++;
                
                // Sichere Vergleichsoperation
                int result = 0;
                try {
                    result = (*entry_i == *entry_j);
                    if (result < 0 || result > 2) {
                        result = 0; // Sicherer Fallback
                    }
                } catch (...) {
                    result = 0; // Sicherer Fallback bei Ausnahmen
                }
                
                goto *jumpTable[result];
                
            ismatch:
                // Match speichern - ohne Begrenzung pro Element
                if (match_count < max_possible_matches) {
                    matches[match_count].data[0] = id_i;
                    matches[match_count].data[1] = id_j;
                    match_count++;
                }
                continue;
                
            nomatch:
                continue;
                
            fallback:
                double similarity = (*entry_i | *entry_j);
                if (similarity >= 0.85 && match_count < max_possible_matches) {
                    // Match speichern - ohne Begrenzung pro Element
                    matches[match_count].data[0] = id_i;
                    matches[match_count].data[1] = id_j;
                    match_count++;
                }
                continue;
            }
        }
        
        // Ergebnisse in den findingsBuffer kopieren
        findingsBuffer->size = match_count;
        if (match_count > 0) {
            findingsBuffer->matches = new match[match_count];
            memcpy(findingsBuffer->matches, matches, match_count * sizeof(match));
            
            // Validierung nach dem Kopieren für zusätzliche Sicherheit
            for (size_t i = 0; i < match_count; i++) {
                if (findingsBuffer->matches[i].data[0] == 0 || findingsBuffer->matches[i].data[1] == 0) {
                    fprintf(stderr, "Error: Invalid match found after copy at index %zu\n", i);
                }
            }
        } else {
            findingsBuffer->matches = nullptr;
        }
        
        // Aufräumen
        delete[] matches;
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
            size_t* offsets = new size_t[numThreads + 1];
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