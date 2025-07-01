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

template<typename compType>
class Matching_mngr {
public:
    Matching_mngr() {}
    ~Matching_mngr() {}   

    void match_blocker_intern(partition* part, size_t start, size_t end, matching* findingsBuffer)
    {
        // Jump-Table für die verschiedenen Ergebnisse des == Operators (0=Match, 1=No Match, 2=Fallback)
        static void* jumpTable[3] =  
        {
            &&ismatch, &&nomatch, &&fallback
        };

        // Zähler für die gefundenen Matches
        unsigned int match_count = 0;
        
        // Maximale Kapazität des Buffers berechnen (worst case: jedes Paar ist ein Match)
        size_t range_size = end - start;
        size_t max_possible_matches = (range_size * (range_size - 1)) / 2;
        
        // Temp-Puffer für die Matches erstellen
        match* matches = (match*)malloc(max_possible_matches * sizeof(match));
        if (!matches) {
            printf("Error: Memory allocation failed in match_blocker_intern\n");
            exit(1);
        }
        
        for(size_t i = start; i < end; i++)
        {
            // Die ID und Daten des aktuellen Elements extrahieren
            uintptr_t id_i = (uintptr_t)part->data[i][0];
            const compType* entry_i = reinterpret_cast<compType*>(part->data[i][1]);
            
            for(size_t j = i+1; j < end; ++j)
            {
                // Die ID und Daten des zu vergleichenden Elements extrahieren
                uintptr_t id_j = (uintptr_t)part->data[j][0];
                const compType* entry_j = reinterpret_cast<compType*>(part->data[j][1]);
                
                // Vergleich durchführen und zum entsprechenden Code springen (0=Match, 1=No Match, 2=Fallback)
                int result = (*entry_i == *entry_j);
                goto *jumpTable[result];
                
            ismatch:
                // Eintrag in Puffer erstellen und speichern
                if (match_count < max_possible_matches) {
                    matches[match_count].data[0] = id_i;
                    matches[match_count].data[1] = id_j;
                    match_count++;
                } else {
                    printf("Warning: Match buffer overflow in match_blocker_intern\n");
                }
                continue; // Zum nächsten Element fortfahren
                
            nomatch:
                // Weitermachen ohne Eintrag zu erstellen
                continue; // Zum nächsten Element fortfahren
                
            fallback:
                // Erweiterter Vergleich für Grenzfälle, bei denen der einfache Vergleich nicht ausreicht
                // Beispiel: Detaillierte Analyse der Produktattribute
                
                // 1. Überprüfe Ähnlichkeit zwischen den Produkten with operator|
                double similarity = (*entry_i | *entry_j);
                
                // 2. Auf Basis der Ähnlichkeit entscheiden
                if (similarity >= 0.85) {  // Schwellwert für Ähnlichkeit
                    // Als Match behandeln, wenn die Ähnlichkeit hoch genug ist
                    if (match_count < max_possible_matches) {
                        matches[match_count].data[0] = id_i;
                        matches[match_count].data[1] = id_j;
                        match_count++;
                    } else {
                        printf("Warning: Match buffer overflow in match_blocker_intern\n");
                    }
                }
                
                // Sonst: Als No-Match behandeln
                continue;
            }
        }
        
        // Ergebnisse in den findingsBuffer kopieren
        findingsBuffer->size = match_count;
        if (match_count > 0) {
            findingsBuffer->matches = (match*)malloc(match_count * sizeof(match));
            if (!findingsBuffer->matches) {
                printf("Error: Memory allocation failed for findings buffer\n");
                free(matches);
                exit(1);
            }
            memcpy(findingsBuffer->matches, matches, match_count * sizeof(match));
        } else {
            findingsBuffer->matches = nullptr;
        }
        
        // Temporären Puffer freigeben
        free(matches);
    }

    dataSet<matching>* identify_matches(dataSet<partition>* input, size_t numThreads)
    {
        size_t num_partitions = input->size;
        matching* matching_buffer = new matching[num_partitions];
        
        // Multithreaded Verarbeitung aller Partitionen
        std::vector<std::thread> threads;
        threads.reserve(num_partitions);
        
        for(int i = 0; i < num_partitions; i++)
        {
            // Thread für jede Partition starten
            threads.emplace_back([this, &input, &matching_buffer, i, numThreads]() {
                partition* part = &input->data[i];
                size_t partition_size = part->size;
                
                // Keine Matches möglich bei zu kleinen Partitionen
                if (partition_size <= 1) {
                    matching_buffer[i].size = 0;
                    matching_buffer[i].matches = nullptr;
                    return;
                }
                
                // Berechne optimale Thread-Aufteilung für quadratischen Vergleich
                // Arbeitsbereich für jeden Thread berechnen, damit jeder etwa gleich viele Vergleiche macht
                size_t total_comparisons = (partition_size * (partition_size - 1)) / 2;
                size_t comparisons_per_thread = total_comparisons / numThreads;
                
                // Array für Thread-Positionen erstellen
                size_t* thread_positions = new size_t[numThreads + 1];
                thread_positions[0] = 0;  // Start bei 0
                thread_positions[numThreads] = partition_size;  // Ende bei partition_size
                
                // Berechne optimale Schnitte für gleichmäßige Thread-Auslastung
                size_t completed_comps = 0;
                for (size_t t = 1; t < numThreads; t++) {
                    // Finde Position, bei der die Anzahl der Vergleiche ungefähr gleich ist
                    size_t i = thread_positions[t-1];
                    size_t remaining_rows = partition_size - i;
                    
                    while (i < partition_size - 1) {
                        size_t comps_in_row = partition_size - i - 1;
                        if (completed_comps + comps_in_row >= comparisons_per_thread * t) {
                            // Finde optimale Position in dieser Zeile
                            size_t pos_in_row = comparisons_per_thread * t - completed_comps;
                            if (pos_in_row < comps_in_row) {
                                thread_positions[t] = i + 1;
                                break;
                            }
                        }
                        completed_comps += comps_in_row;
                        i++;
                        thread_positions[t] = i;
                    }
                }
                
                // Threads für jeden Bereich erstellen und ausführen
                std::vector<std::thread> partition_threads;
                std::vector<matching> thread_results(numThreads);
                
                for (size_t t = 0; t < numThreads; t++) {
                    size_t start = thread_positions[t];
                    size_t end = thread_positions[t+1];
                    
                    if (start < end) {
                        partition_threads.emplace_back([this, part, start, end, &thread_results, t]() {
                            // Rufe interne Matching-Funktion für den Bereich auf
                            match_blocker_intern(part, start, end, &thread_results[t]);
                        });
                    }
                }
                
                // Warte auf Abschluss aller Threads
                for (auto& t : partition_threads) {
                    if (t.joinable()) {
                        t.join();
                    }
                }
                
                // Ergebnisse zusammenführen
                size_t total_matches = 0;
                for (size_t t = 0; t < numThreads; t++) {
                    total_matches += thread_results[t].size;
                }
                
                // Ergebnispuffer allozieren und Matches zusammenführen
                matching_buffer[i].size = total_matches;
                if (total_matches > 0) {
                    matching_buffer[i].matches = (match*)malloc(total_matches * sizeof(match));
                    if (!matching_buffer[i].matches) {
                        printf("Error: Memory allocation failed in identify_matches\n");
                        exit(1);
                    }
                    
                    size_t offset = 0;
                    for (size_t t = 0; t < numThreads; t++) {
                        if (thread_results[t].size > 0) {
                            memcpy(matching_buffer[i].matches + offset, 
                                  thread_results[t].matches, 
                                  thread_results[t].size * sizeof(match));
                            offset += thread_results[t].size;
                            
                            // Speicher freigeben
                            free(thread_results[t].matches);
                            thread_results[t].matches = nullptr;
                        }
                    }
                } else {
                    matching_buffer[i].matches = nullptr;
                }
                
                // Aufräumen
                delete[] thread_positions;
            });
        }
        
        // Warte auf Abschluss aller Partition-Threads
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        
        // Ergebnisse in dataSet verpacken
        dataSet<matching>* result = new dataSet<matching>();
        result->size = num_partitions;
        result->data = matching_buffer;
        
        return result;
    }
    
    // Einfachere Variante des Matchers, die einen vordefinierten Buffer verwendet
    // Diese Funktion schreibt Matches direkt in einen vordefinierten Puffer und gibt die Anzahl der gefundenen Matches zurück
    size_t match_range_simple(partition* part, size_t start, size_t end, match* match_buffer, size_t buffer_capacity)
    {
        // Jump-Table für die verschiedenen Ergebnisse des == Operators (0=Match, 1=No Match, 2=Fallback)
        static void* jumpTable[3] =  
        {
            &&ismatch, &&nomatch, &&fallback
        };

        // Zähler für die gefundenen Matches
        size_t match_count = 0;
        
        for(size_t i = start; i < end; i++)
        {
            // Die ID und Daten des aktuellen Elements extrahieren
            uintptr_t id_i = (uintptr_t)part->data[i][0];
            const compType* entry_i = reinterpret_cast<compType*>(part->data[i][1]);
            
            for(size_t j = i+1; j < end; ++j)
            {
                // Die ID und Daten des zu vergleichenden Elements extrahieren
                uintptr_t id_j = (uintptr_t)part->data[j][0];
                const compType* entry_j = reinterpret_cast<compType*>(part->data[j][1]);
                
                // Vergleich durchführen und zum entsprechenden Code springen (0=Match, 1=No Match, 2=Fallback)
                int result = (*entry_i == *entry_j);
                goto *jumpTable[result];
                
            ismatch:
                // Eintrag in Puffer erstellen und speichern
                if (match_count < buffer_capacity) {
                    match_buffer[match_count].data[0] = id_i;
                    match_buffer[match_count].data[1] = id_j;
                    match_count++;
                } else {
                    printf("Warning: Match buffer overflow in match_range_simple\n");
                    return match_count; // Puffer voll, beende frühzeitig
                }
                continue; // Zum nächsten Element fortfahren
                
            nomatch:
                // Weitermachen ohne Eintrag zu erstellen
                continue; // Zum nächsten Element fortfahren
                
            fallback:
                // Erweiterter Vergleich für Grenzfälle
                double similarity = (*entry_i | *entry_j);
                
                if (similarity >= 0.85) {  // Schwellwert für Ähnlichkeit
                    if (match_count < buffer_capacity) {
                        match_buffer[match_count].data[0] = id_i;
                        match_buffer[match_count].data[1] = id_j;
                        match_count++;
                    } else {
                        printf("Warning: Match buffer overflow in match_range_simple\n");
                        return match_count; // Puffer voll, beende frühzeitig
                    }
                }
                continue;
            }
        }
        
        return match_count;
    }

    // Parallele Verarbeitung einer einzelnen Partition mit optimierter Pufferverwaltung
    matching match_partition_simple_parallel(partition* part, size_t numThreads)
    {
        size_t partition_size = part->size;
        matching result = {nullptr, 0};
        
        // Keine Matches möglich bei zu kleinen Partitionen
        if (partition_size <= 1) {
            return result;
        }
        
        // Berechne optimale Thread-Aufteilung für quadratischen Vergleich
        size_t total_comparisons = (partition_size * (partition_size - 1)) / 2;
        size_t comparisons_per_thread = total_comparisons / numThreads;
        
        // Array für Thread-Positionen erstellen
        size_t* thread_positions = new size_t[numThreads + 1];
        thread_positions[0] = 0;
        thread_positions[numThreads] = partition_size;
        
        // Berechne optimale Schnitte für gleichmäßige Thread-Auslastung
        size_t completed_comps = 0;
        for (size_t t = 1; t < numThreads; t++) {
            size_t i = thread_positions[t-1];
            
            while (i < partition_size - 1) {
                size_t comps_in_row = partition_size - i - 1;
                if (completed_comps + comps_in_row >= comparisons_per_thread * t) {
                    // Finde optimale Position in dieser Zeile
                    size_t pos_in_row = comparisons_per_thread * t - completed_comps;
                    if (pos_in_row < comps_in_row) {
                        thread_positions[t] = i + 1;
                        break;
                    }
                }
                completed_comps += comps_in_row;
                i++;
                thread_positions[t] = i;
            }
        }
        
        // Schätzen der maximalen Anzahl von Matches (worst case: alle Vergleiche sind Matches)
        size_t estimated_max_matches = total_comparisons;
        
        // Erstelle einen großen Puffer für alle Threads
        struct ThreadResult {
            match* buffer;
            size_t capacity;
            size_t count;
        };
        std::vector<ThreadResult> thread_results(numThreads);
        
        // Teile den Puffer in Abschnitte für jeden Thread
        size_t buffer_per_thread = estimated_max_matches / numThreads + 1; // +1 für Rundungsfehler
        
        for (size_t t = 0; t < numThreads; t++) {
            thread_results[t].buffer = new match[buffer_per_thread];
            thread_results[t].capacity = buffer_per_thread;
            thread_results[t].count = 0;
        }
        
        // Erstelle und starte Threads
        std::vector<std::thread> threads;
        for (size_t t = 0; t < numThreads; t++) {
            size_t start = thread_positions[t];
            size_t end = thread_positions[t+1];
            
            if (start < end) {
                threads.emplace_back([this, part, start, end, &thread_results, t]() {
                    // Verwende die einfachere Matching-Funktion mit voralloziertem Puffer
                    thread_results[t].count = match_range_simple(
                        part, 
                        start, 
                        end, 
                        thread_results[t].buffer, 
                        thread_results[t].capacity
                    );
                });
            }
        }
        
        // Warte auf Abschluss aller Threads
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        
        // Ergebnisse zusammenführen
        size_t total_matches = 0;
        for (size_t t = 0; t < numThreads; t++) {
            total_matches += thread_results[t].count;
        }
        
        // Ergebnispuffer allozieren und Matches zusammenführen
        if (total_matches > 0) {
            result.size = total_matches;
            result.matches = (match*)malloc(total_matches * sizeof(match));
            if (!result.matches) {
                printf("Error: Memory allocation failed in match_partition_simple_parallel\n");
                
                // Speicher freigeben
                for (size_t t = 0; t < numThreads; t++) {
                    delete[] thread_results[t].buffer;
                }
                delete[] thread_positions;
                
                exit(1);
            }
            
            size_t offset = 0;
            for (size_t t = 0; t < numThreads; t++) {
                if (thread_results[t].count > 0) {
                    memcpy(
                        result.matches + offset, 
                        thread_results[t].buffer, 
                        thread_results[t].count * sizeof(match)
                    );
                    offset += thread_results[t].count;
                }
            }
        }
        
        // Speicher freigeben
        for (size_t t = 0; t < numThreads; t++) {
            delete[] thread_results[t].buffer;
        }
        delete[] thread_positions;
        
        return result;
    }
    
    // Verarbeite alle Partitionen parallel mit der einfachen Puffermethode
    dataSet<matching>* match_all_partitions_simple(dataSet<partition>* input, size_t numThreads)
    {
        size_t num_partitions = input->size;
        matching* matching_buffer = new matching[num_partitions];
        
        // Parallele Verarbeitung der Partitionen
        #pragma omp parallel for num_threads(numThreads) if(num_partitions > 1)
        for(int i = 0; i < num_partitions; i++) {
            // Verarbeite jede Partition parallel intern
            matching_buffer[i] = match_partition_simple_parallel(&input->data[i], numThreads);
        }
        
        // Ergebnisse in dataSet verpacken
        dataSet<matching>* result = new dataSet<matching>();
        result->size = num_partitions;
        result->data = matching_buffer;
        
        return result;
    }
};

#endif