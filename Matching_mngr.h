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
    // Klassenvariable für die gemeinsamen Match-Counts über alle Threads
    std::atomic<size_t>* shared_match_counts;
    size_t shared_match_counts_size;

public:
    Matching_mngr() : shared_match_counts(nullptr), shared_match_counts_size(0) {}
    
    ~Matching_mngr() {
        // Sicherstellen, dass shared_match_counts freigegeben wird
        freeSharedMatchCounts();
    }
    
    // Allokieren der shared_match_counts mit einer bestimmten Größe
    void allocateSharedMatchCounts(size_t size) {
        // Erst freigeben, falls bereits allokiert
        freeSharedMatchCounts();
        
        // Neu allokieren und initialisieren
        shared_match_counts_size = size;
        shared_match_counts = new std::atomic<size_t>[size]();
        
        // Explizites Initialisieren aller Elemente mit 0
        for (size_t i = 0; i < size; ++i) {
            shared_match_counts[i] = 0;
        }
    }
    
    // Freigeben der shared_match_counts
    void freeSharedMatchCounts() {
        if (shared_match_counts != nullptr) {
            delete[] shared_match_counts;
            shared_match_counts = nullptr;
            shared_match_counts_size = 0;
        }
    }
    
    // Zurücksetzen aller Zähler auf 0
    void resetSharedMatchCounts() {
        if (shared_match_counts != nullptr) {
            for (size_t i = 0; i < shared_match_counts_size; ++i) {
                shared_match_counts[i] = 0;
            }
        }
    }
   
    // Thread-sicheres Matching mit Atomics - verwendet jetzt die Klassenvariable
    void match_blocker_intern(partition* part, size_t start, size_t end, matching* findingsBuffer)
    {
        // Initialisiere das findingsBuffer-Objekt sofort, um uninitialisierte Werte zu vermeiden
        if (findingsBuffer) {
            findingsBuffer->size = 0;
            findingsBuffer->matches = nullptr;
        }
        
        // Jump-Table für die verschiedenen Ergebnisse des == Operators (0=No Match, 1=Match, 2=Fallback)
        static void *jumpTable[3] =
        {
            &&nomatch, &&ismatch, &&fallback
        };
        
        // Jump-Tables für Element-Match-Limits
        static void *itemLimitJumpTable_i[MAX_MATCHES_PER_ITEM + 1] = 
        {
            &&check_item_i, &&check_item_i, &&check_item_i, &&skip_item_i
        };
        
        static void *itemLimitJumpTable_j[MAX_MATCHES_PER_ITEM + 1] = 
        {
            &&check_item_j, &&check_item_j, &&check_item_j, &&skip_item_j
        };

        // Zähler für die gefundenen Matches
        unsigned int match_count = 0;
        
        // Sicherstellen, dass die Eingabeparameter gültig sind
        if (part == nullptr || part->data == nullptr || findingsBuffer == nullptr || start >= end || this->shared_match_counts == nullptr) {
            fprintf(stderr, "Error: Invalid parameters in match_blocker_intern: part=%p, findingsBuffer=%p, start=%zu, end=%zu, shared_match_counts=%p\n", 
                  part, findingsBuffer, start, end, this->shared_match_counts);
            return;
        }
        
        // Überprüfen der Arraygröße
        if (end > this->shared_match_counts_size) {
            fprintf(stderr, "Error: End index (%zu) exceeds shared_match_counts size (%zu)\n", 
                   end, this->shared_match_counts_size);
            return;
        }
        
        // Debug-Output für Arraygröße und Speicherzugriffe
       
        
        // Maximale Kapazität des Buffers berechnen (worst case: jedes Paar ist ein Match)
        size_t range_size = end - start;
        size_t max_possible_matches = (range_size * (range_size - 1)) / 2;
        
        // Temp-Puffer für die Matches erstellen und mit Nullen initialisieren
        match* matches = new match[max_possible_matches];
        if (!matches) 
        {
            fprintf(stderr, "Error: Memory allocation failed in match_blocker_intern\n");
            exit(1);
        }
        
        // Explizit alle Matches initialisieren (für erhöhte Sicherheit)
        for (size_t i = 0; i < max_possible_matches; i++) {
            matches[i].data[0] = 0;
            matches[i].data[1] = 0;
        }
        
        size_t comparison_count = 0;
        size_t skipped_comparisons = 0;
        
        for(size_t i = start; i < end; i++)
        {
            // Überprüfe shared_match_counts bevor darauf zugegriffen wird
            if (shared_match_counts == nullptr) {
                fprintf(stderr, "Error: shared_match_counts is NULL at i=%zu\n", i);
                return;
            }
            
            // Überprüfe, ob der Index gültig ist
            if (i >= shared_match_counts_size) {
                fprintf(stderr, "Error: Index i=%zu exceeds shared_match_counts_size=%zu\n", i, shared_match_counts_size);
                return;
            }
            
            // Verwende die Klassenvariable anstatt des Parameters
            size_t count_i = shared_match_counts[i].load(std::memory_order_acquire);
            
            if (count_i >= MAX_MATCHES_PER_ITEM) {
                goto skip_item_i;
            }
            
            goto *itemLimitJumpTable_i[count_i];
            
        skip_item_i:
            skipped_comparisons += (end - i - 1);
            continue;
            
        check_item_i:                              
                // Debug: Überprüfen Sie die Gültigkeit von part->data[i][0] und part->data[i][1]
                if (!part->data[i][1]) {
                    fprintf(stderr, "ERROR: part->data[%zu][1] is NULL\n", i);
                    return;
                }
                
                // Debug: Überprüfen Sie die Gültigkeit des compType-Objekts
                const compType* debug_entry = reinterpret_cast<compType*>(part->data[i][1]);

            uintptr_t id_i = (uintptr_t)part->data[i][0];
            const compType* entry_i = reinterpret_cast<compType*>(part->data[i][1]);
            
            for(size_t j = i+1; j < end; ++j)
            {
                // Überprüfe shared_match_counts bevor darauf zugegriffen wird
                if (shared_match_counts == nullptr) {
                    fprintf(stderr, "Error: shared_match_counts is NULL at j=%zu\n", j);
                    return;
                }
                
                // Überprüfe, ob der Index gültig ist
                if (j >= shared_match_counts_size) {
                    fprintf(stderr, "Error: Index j=%zu exceeds shared_match_counts_size=%zu\n", j, shared_match_counts_size);
                    return;
                }
                
                size_t count_j = shared_match_counts[j].load(std::memory_order_acquire);
                
                goto *itemLimitJumpTable_j[count_j];

                skip_item_j:
                    skipped_comparisons++;
                    continue;

                check_item_j:
                    if (j >= part->size) 
                    {
                        fprintf(stderr, "ERROR: j=%zu is out of bounds (part->size=%zu)\n", j, part->size);
                        return;
                    }

                    // Debug: Überprüfe part->data[j][1]
                
                    if (part->data[j][1] == 0) {
                        fprintf(stderr, "ERROR: part->data[%zu][1] is NULL\n", j);
                        return;
                    }

                    // Debug: Überprüfen Sie die Gültigkeit des compType-Objekts
                    const compType* debug_entry_j = reinterpret_cast<compType*>(part->data[j][1]);
                    if (debug_entry_j == nullptr) {
                        fprintf(stderr, "ERROR: reinterpret_cast<compType*>(part->data[%zu][1]) is NULL\n", j);
                        return;
                    }

                    uintptr_t id_j = (uintptr_t)part->data[j][0];
                    const compType* entry_j = reinterpret_cast<compType*>(part->data[j][1]);

                    comparison_count++;

                    // Sichere Vergleichsoperation mit Fehlerbehandlung
                    int result = 0;
                    try {
                        //hier findet der centrale vergleich statt ===========================================================================================
                        result = (*entry_i == *entry_j);
                        // Debug-Ausgabe bei jedem Vergleich (nur in Debug-Builds)

                        // Wenn das Ergebnis außerhalb des gültigen Bereichs liegt
                        if (result < 0 || result > 2) {
                            fprintf(stderr, "WARNING: Invalid comparison result: %d (id_i=%zu, id_j=%zu)\n", 
                                    result, static_cast<size_t>(id_i), static_cast<size_t>(id_j));
                            result = 0; // Sicherer Fallback auf "kein Match"
                        }
                    } catch (...) {
                        // Fehlerbehandlung für Ausnahmen während des Vergleichs
                        fprintf(stderr, "ERROR: Exception during comparison (id_i=%zu, id_j=%zu)\n", 
                                static_cast<size_t>(id_i), static_cast<size_t>(id_j));
                        result = 0; // Sicherer Fallback auf "kein Match"
                    }

                    goto *jumpTable[result];

                ismatch:
                    printf("\t [%zu == %zu]\n", static_cast<size_t>(id_i), static_cast<size_t>(id_j));
                    if (match_count < max_possible_matches) 
                    {
                        matches[match_count].data[0] = id_i;
                        matches[match_count].data[1] = id_j;
                        match_count++;

                        bool can_increment_i = false;
                        bool can_increment_j = false;

                        size_t current_i, current_j;

                        // Für Element i - verwende die Klassenvariable
                        do {
                            current_i = shared_match_counts[i].load(std::memory_order_acquire);
                            if (current_i >= MAX_MATCHES_PER_ITEM) {
                                can_increment_i = false;
                                break;
                            }
                            can_increment_i = shared_match_counts[i].compare_exchange_strong(
                                current_i, current_i + 1, std::memory_order_acq_rel);
                        } while (!can_increment_i);

                        // Für Element j - verwende die Klassenvariable
                        do {
                            current_j = shared_match_counts[j].load(std::memory_order_acquire);
                            if (current_j >= MAX_MATCHES_PER_ITEM) {
                                can_increment_j = false;
                                break;
                            }
                            can_increment_j = shared_match_counts[j].compare_exchange_strong(
                                current_j, current_j + 1, std::memory_order_acq_rel);
                        } while (!can_increment_j);
                    } 
                    continue;

                nomatch:
                    continue;

                fallback:
                    printf("\t [%zu ~~ %zu]\n", static_cast<size_t>(id_i), static_cast<size_t>(id_j));
                    double similarity = (*entry_i | *entry_j);

                    if (similarity >= 0.85)
                    {  
                        if (match_count < max_possible_matches) 
                        {
                            matches[match_count].data[0] = id_i;
                            matches[match_count].data[1] = id_j;
                            match_count++;

                            bool can_increment_i = false;
                            bool can_increment_j = false;

                            size_t current_i, current_j;

                            // Verwende die Klassenvariable für Element i
                            while (!can_increment_i)
                            {
                                current_i = shared_match_counts[i].load(std::memory_order_acquire);
                                if (current_i >= MAX_MATCHES_PER_ITEM) {
                                    can_increment_i = false;
                                    break;
                                }
                                can_increment_i = shared_match_counts[i].compare_exchange_strong(
                                    current_i, current_i + 1, std::memory_order_acq_rel);
                            };

                            // Verwende die Klassenvariable für Element j
                            while (!can_increment_j)
                            {
                                current_j = shared_match_counts[j].load(std::memory_order_acquire);
                                if (current_j >= MAX_MATCHES_PER_ITEM) 
                                {
                                    can_increment_j = false;
                                    break;
                                }
                                can_increment_j = shared_match_counts[j].compare_exchange_strong(
                                    current_j, current_j + 1, std::memory_order_acq_rel);
                            };
                        } 
                    }
                    continue;
                }
        }   
        
        // Ergebnisse in den findingsBuffer kopieren
        findingsBuffer->size = match_count;
        if (match_count > 0) 
        {
            findingsBuffer->matches = new match[match_count];
            if (!findingsBuffer->matches) 
            {
                fprintf(stderr, "Error: Memory allocation failed for findings buffer\n");
                delete[] matches;
                exit(1);
            }
                   
            memcpy(findingsBuffer->matches, matches, match_count * sizeof(match));
        } 
        else 
        {
            findingsBuffer->matches = nullptr;
        }
        
        // Temporären Puffer freigeben
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
        // Initialisiere den matching_buffer mit Nullen
        matching* matching_buffer = new matching[num_partitions]();
        
        for(size_t p = 0; p < num_partitions; p++)
        {
            if (input->data == nullptr) {
                fprintf(stderr, "Error: Partition data is null at index %zu\n", p);
                continue;
            }
            
            partition* part = &input->data[p];
            
            // Sicherstellen, dass die Partitionsgröße gültig ist
            if (part == nullptr || part->data == nullptr) {
                matching_buffer[p].size = 0;
                matching_buffer[p].matches = nullptr;
                continue;
            }
            
            size_t partition_size = part->size;
            
            if (partition_size <= 1) {
                matching_buffer[p].size = 0;
                matching_buffer[p].matches = nullptr;
                continue;
            }
            
            // Shared match counts für diese Partition allokieren und zurücksetzen
            allocateSharedMatchCounts(partition_size);
            
            if (partition_size < 50 || numThreads == 1) {
                match_blocker_intern(part, 0, partition_size, &matching_buffer[p]);
                continue;
            }
            
            // Array zur Verfolgung von Elementen mit vielen Matches (für Debug-Zwecke)
            std::atomic<bool>* debug_track_high_matches = new std::atomic<bool>[partition_size]();
            
            size_t total_comparisons = (partition_size * (partition_size - 1)) / 2;
            size_t comparisons_per_thread = total_comparisons / numThreads;
            
            // Array für Thread-Positionen erstellen
            size_t* offsets = new size_t[numThreads + 1];
            memset(offsets, 0, sizeof(size_t) * (numThreads + 1));
            offsets[0] = 0;
            offsets[numThreads] = partition_size;
            
            // Berechne optimale Schnitte für gleichmäßige Thread-Auslastung (quadratisch)
            size_t completed_comps = 0;
            for (size_t t = 1; t < numThreads; t++) {
                size_t i = offsets[t-1];
                
                while (i < partition_size - 1) 
                {
                    size_t comps_in_row = partition_size - i - 1;
                    if (completed_comps + comps_in_row >= comparisons_per_thread * t) 
                    {
                        size_t pos_in_row = comparisons_per_thread * t - completed_comps;
                        if (pos_in_row < comps_in_row)
                        {
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
            std::thread** threads = new std::thread*[numThreads]();  // Mit Nullen initialisieren
            matching* thread_results = new matching[numThreads]();   // Mit Nullen initialisieren
            
            // Initialisiere alle Thread-Ergebnisse explizit
            for (size_t t = 0; t < numThreads; t++) {
                thread_results[t].size = 0;
                thread_results[t].matches = nullptr;
            }
            
            // Starte die Threads für die jeweiligen Bereiche - jetzt ohne shared_match_counts als Parameter
            for (size_t t = 0; t < numThreads; t++) 
            {
                size_t start = offsets[t];
                size_t end = offsets[t+1];
                
                if (start < end && end <= partition_size)  // Sicherstellen, dass die Grenzen gültig sind
                {
                    // Stellen sicher, dass shared_match_counts direkt als Referenz im Lambda erfasst wird
                    // um Race Conditions beim Zugriff über 'this' zu vermeiden
                    std::atomic<size_t>* current_shared_counts = this->shared_match_counts;

                    threads[t] = new std::thread([this, part, start, end, &thread_results, t, numThreads, current_shared_counts]()
                    {
                        // Thread-lokale Kopie von shared_match_counts verwenden
                        if (part && part->data && current_shared_counts) {
                            // Sicherstellen, dass shared_match_counts auch im Thread verfügbar ist
                            this->shared_match_counts = current_shared_counts;
                            match_blocker_intern(part, start, end, &thread_results[t]);
                        } else {
                            fprintf(stderr, "Thread %zu: Invalid parameters\n", t+1);
                            thread_results[t].size = 0;
                            thread_results[t].matches = nullptr;
                        }
                    });
                } 
                else 
                {
                    threads[t] = nullptr;
                }
            }
            
            // Warte auf Abschluss aller Threads
            for (size_t t = 0; t < numThreads; t++) {
                if (threads[t] && threads[t]->joinable()) {
                    threads[t]->join();
                    delete threads[t];
                    threads[t] = nullptr;  // Pointer auf null setzen nach dem Löschen
                }
            }
            delete[] threads;
            threads = nullptr;
            
            // Ergebnisse zusammenführen
            size_t total_matches = 0;
            for (size_t t = 0; t < numThreads; t++) {
                total_matches += thread_results[t].size;
            }
            
            // Ergebnispuffer für die Partition allozieren und Matches zusammenführen
            matching_buffer[p].size = total_matches;
            if (total_matches > 0) {
                matching_buffer[p].matches = new match[total_matches]();  // Mit Nullen initialisieren
                if (!matching_buffer[p].matches) {
                    fprintf(stderr, "Error: Memory allocation failed in identify_matches\n");
                    delete[] offsets;
                    delete[] thread_results;
                    freeSharedMatchCounts();  // Shared match counts freigeben vor dem Beenden
                    exit(1);
                }
                
                size_t offset = 0;
                for (size_t t = 0; t < numThreads; t++) {
                    if (thread_results[t].size > 0) {
                        memcpy(matching_buffer[p].matches + offset, 
                              thread_results[t].matches, 
                              thread_results[t].size * sizeof(match));
                        offset += thread_results[t].size;
                        
                        // Speicher freigeben
                        delete[] thread_results[t].matches;
                        thread_results[t].matches = nullptr;
                    }
                }
            } else {
                matching_buffer[p].matches = nullptr;
            }
            
           
            
            // Statistik über die Verteilung der Matches ausgeben
            size_t max_matches = 0;
            size_t elements_at_limit = 0;
            size_t histogram[MAX_MATCHES_PER_ITEM+1] = {0};
            
            // Sicherheitscheck, ob shared_match_counts noch gültig ist
            if (shared_match_counts ) 
            {
                // Überprüfe ob shared_match_counts_size gültig ist
                if (partition_size > shared_match_counts_size) 
                {
                    partition_size = shared_match_counts_size; // Auf sichere Größe begrenzen
                }
                
                for (size_t i = 0; i < partition_size; i++) {
                    // Sicheres Laden des Wertes
                    size_t count = shared_match_counts[i].load(std::memory_order_acquire);
                    
                    max_matches = std::max(max_matches, count);
                    if (count == MAX_MATCHES_PER_ITEM) {
                        elements_at_limit++;
                    }
                    
                    if (count <= MAX_MATCHES_PER_ITEM) {
                        histogram[count]++;
                    }
                }
            }

            // Aufräumen
            delete[] offsets;
            delete[] thread_results;
            delete[] debug_track_high_matches;
        }
        
        // Zum Schluss die Klassenvariable freigeben
        freeSharedMatchCounts();
        
        // Ergebnisse in dataSet verpacken
        dataSet<matching>* result = new dataSet<matching>();
        result->size = num_partitions;
        result->data = matching_buffer;
        
        return result;
    }
};
#endif