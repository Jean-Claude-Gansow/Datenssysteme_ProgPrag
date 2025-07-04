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
class Matching_mngr 
{
    

public:
    Matching_mngr() {}
    ~Matching_mngr() {}   
   
    

    // Thread-sicheres Matching mit Atomics
    void match_blocker_intern(partition* part, size_t start, size_t end, matching* findingsBuffer)
    {
        // Jump-Table für die verschiedenen Ergebnisse des == Operators (0=Match, 1=No Match, 2=Fallback)
        static void *jumpTable[3] =
        {
            &&nomatch, &&ismatch, &&fallback
        };
        
        // Verwende die globale MAX_MATCHES_PER_ITEM Konstante
        
        // Jump-Tables für Element-Match-Limits
        // Diese Tabellen haben MAX_MATCHES_PER_ITEM+1 Einträge
        // Alle Indizes < MAX_MATCHES_PER_ITEM zeigen auf "check_item_X"
        // Nur der letzte Eintrag (MAX_MATCHES_PER_ITEM) zeigt auf "skip_item_X"
        static void *itemLimitJumpTable_i[MAX_MATCHES_PER_ITEM + 1] = 
        {
            &&check_item_i, &&check_item_i, &&check_item_i, &&skip_item_i
        };
        
        // Für das zweite Element: bei Überschreitung der maximalen Matches zum nächsten j springen
        static void *itemLimitJumpTable_j[MAX_MATCHES_PER_ITEM + 1] = 
        {
            &&check_item_j, &&check_item_j, &&check_item_j, &&skip_item_j
        };

        // Zähler für die gefundenen Matches
        unsigned int match_count = 0;
        
        // Maximale Kapazität des Buffers berechnen (worst case: jedes Paar ist ein Match)
        size_t range_size = end - start;
        size_t max_possible_matches = (range_size * (range_size - 1)) / 2;
        
        // Temp-Puffer für die Matches erstellen
        match* matches = new match[max_possible_matches];
        if (!matches) 
        {
            fprintf(stderr, "Error: Memory allocation failed in match_blocker_intern\n");
            exit(1);
        }
        
        // Array zur Verfolgung der Match-Anzahl pro Element
        // Dadurch können wir Elemente überspringen, die bereits genügend Matches haben
        
        size_t comparison_count = 0;
        size_t skipped_comparisons = 0;
        
        for(size_t i = start; i < end; i++)
        {
            
            // Überprüfe mit Jump-Table ob wir dieses Element überspringen sollten
            // Die match_count-Werte sollten nie größer als MAX_MATCHES_PER_ITEM sein
            // Mit stärkerem Memory-Ordering, um sicherzustellen, dass wir den aktuellen Wert sehen
            size_t count_i = atomic_match_counts[i].load(std::memory_order_acquire);
            
            // Sicherheitsüberprüfung und Sprung
            if (count_i >= MAX_MATCHES_PER_ITEM) {
                DEBUG_MATCH_DETAILED("Element %zu wird übersprungen (bereits %zu Matches)\n", i, count_i);
                goto skip_item_i;
            }
            
            // Direkte Verwendung des Zählerwerts als Index
            goto *itemLimitJumpTable_i[count_i];
            
        skip_item_i:
            // Element i hat bereits genug Matches, überspringe alle verbleibenden Vergleiche
            skipped_comparisons += (end - i - 1);
            continue;
            
        check_item_i:
            // Die ID und Daten des aktuellen Elements extrahieren
            uintptr_t id_i = (uintptr_t)part->data[i][0];
            const compType* entry_i = reinterpret_cast<compType*>(part->data[i][1]);
            
            for(size_t j = i+1; j < end; ++j)
            {
                // Kein lokaler Index mehr notwendig, da wir nur noch mit atomic_match_counts arbeiten
                
                // Überprüfe mit Jump-Table ob wir dieses Element überspringen sollten
                // Mit stärkerem Memory-Ordering, um sicherzustellen, dass wir den aktuellen Wert sehen
                size_t count_j = atomic_match_counts[j].load(std::memory_order_acquire);
                
                // Sicherheitsüberprüfung und Sprung
                if (count_j >= MAX_MATCHES_PER_ITEM) {
                    DEBUG_MATCH_DETAILED("Element %zu wird übersprungen (bereits %zu Matches)\n", j, count_j);
                    goto skip_item_j;
                }
                
                // Direkte Verwendung des Zählerwerts als Index
                goto *itemLimitJumpTable_j[count_j];
                
            skip_item_j: //ignore this comparisson, it has already have enough 
                // Element j hat bereits genug Matches, überspringe diesen Vergleich
                skipped_comparisons++;
                continue;
                
            check_item_j: //proceed to check
                // Die ID und Daten des zu vergleichenden Elements extrahieren
                uintptr_t id_j = (uintptr_t)part->data[j][0];
                const compType* entry_j = reinterpret_cast<compType*>(part->data[j][1]);
                
                comparison_count++;
                
                // Vergleich durchführen und zum entsprechenden Code springen (0=Match, 1=No Match, 2=Fallback)
                int result = (*entry_i == *entry_j);
                goto *jumpTable[result];
                
            ismatch:
                printf("\t [%ld == %ld]\n", i, j);
                // Eintrag in Puffer erstellen und speichern
                if (match_count < max_possible_matches) 
                {
                    matches[match_count].data[0] = id_i;
                    matches[match_count].data[1] = id_j;
                    match_count++;
                    
                    // Match-Anzahl für beide Elemente erhöhen
                    // Überprüfe zuerst, ob das Element bereits genug Matches hat
                    bool can_increment_i = false;
                    bool can_increment_j = false;
                    
                    // Thread-sichere Inkrementierung mit atomaren Operationen
                    size_t current_i, current_j;
                    
                    // Für Element i
                    do {
                        current_i = atomic_match_counts[i].load(std::memory_order_acquire);
                        if (current_i >= MAX_MATCHES_PER_ITEM) {
                            can_increment_i = false;
                            break;  // Bereits genug Matches
                        }
                        can_increment_i = atomic_match_counts[i].compare_exchange_strong(
                            current_i, current_i + 1, std::memory_order_acq_rel);
                    } while (!can_increment_i); // Wiederholen, falls CAS fehlschlägt
                    
                    // Für Element j
                    do {
                        current_j = atomic_match_counts[j].load(std::memory_order_acquire);
                        if (current_j >= MAX_MATCHES_PER_ITEM) {
                            can_increment_j = false;
                            break;  // Bereits genug Matches
                        }
                        can_increment_j = atomic_match_counts[j].compare_exchange_strong(
                            current_j, current_j + 1, std::memory_order_acq_rel);
                    } while (!can_increment_j); // Wiederholen, falls CAS fehlschlägt
                    
                    // Debug-Ausgabe, wenn ein Element das Limit erreicht hat
                    if (!can_increment_i) 
                    {
                        DEBUG_MATCH_DETAILED("Element %zu hat das Match-Limit (%d) erreicht\n", i, MAX_MATCHES_PER_ITEM);
                    }
                    if (!can_increment_j) 
                    {
                        DEBUG_MATCH_DETAILED("Element %zu hat das Match-Limit (%d) erreicht\n", j, MAX_MATCHES_PER_ITEM);
                    }
                } 
                else 
                {
                    DEBUG_MATCH_IMPORTANT("Warning: Match buffer overflow in match_blocker_intern\n");
                }
                continue; // Zum nächsten Element fortfahren
                
            nomatch:
                continue; // Zum nächsten Element fortfahren
                
            fallback:
                printf("\t [%ld ~~ %ld]\n", i, j);
                // Erweiterter Vergleich für Grenzfälle, bei denen der einfache Vergleich nicht ausreicht
                // 1. Überprüfe Ähnlichkeit zwischen den Produkten with operator|
                double similarity = (*entry_i | *entry_j);
                
                // 2. Auf Basis der Ähnlichkeit entscheiden
                if (similarity >= 0.85) // Schwellwert für Ähnlichkeit = 0.85 jaccard index
                {  
                    // Als Match behandeln, wenn die Ähnlichkeit hoch genug ist
                    if (match_count < max_possible_matches) 
                    {
                        matches[match_count].data[0] = id_i;
                        matches[match_count].data[1] = id_j;
                        match_count++;
                        
                        // Match-Anzahl für beide Elemente erhöhen
                        // Überprüfe zuerst, ob das Element bereits genug Matches hat
                        bool can_increment_i = false;
                        bool can_increment_j = false;
                        
                        // Thread-sichere Inkrementierung mit atomaren Operationen
                        size_t current_i, current_j;
                        
                        // Für Element i
                        do {
                            current_i = atomic_match_counts[i].load(std::memory_order_acquire);
                            if (current_i >= MAX_MATCHES_PER_ITEM) {
                                can_increment_i = false;
                                break;  // Bereits genug Matches
                            }
                            can_increment_i = atomic_match_counts[i].compare_exchange_strong(
                                current_i, current_i + 1, std::memory_order_acq_rel);
                        } while (!can_increment_i); // Wiederholen, falls CAS fehlschlägt
                        
                        // Für Element j
                        do {
                            current_j = atomic_match_counts[j].load(std::memory_order_acquire);
                            if (current_j >= MAX_MATCHES_PER_ITEM) {
                                can_increment_j = false;
                                break;  // Bereits genug Matches
                            }
                            can_increment_j = atomic_match_counts[j].compare_exchange_strong(
                                current_j, current_j + 1, std::memory_order_acq_rel);
                        } while (!can_increment_j); // Wiederholen, falls CAS fehlschlägt
                        
                        // Debug-Ausgabe, wenn ein Element das Limit erreicht hat
                        if (!can_increment_i) {
                            DEBUG_MATCH_DETAILED("Element %zu hat das Match-Limit (%d) erreicht (fallback)\n", i, MAX_MATCHES_PER_ITEM);
                        }
                        if (!can_increment_j) {
                            DEBUG_MATCH_DETAILED("Element %zu hat das Match-Limit (%d) erreicht (fallback)\n", j, MAX_MATCHES_PER_ITEM);
                        }
                    } 
                    //else 
                    //{
                    //    DEBUG_MATCH_IMPORTANT("Warning: Match buffer overflow in match_blocker_intern\n");
                    //}
                }
                
                // Sonst: Als No-Match behandeln
                continue;
            }
        }
        
        // Statistiken ausgeben
        //if (skipped_comparisons > 0) {
        //    DEBUG_MATCH_IMPORTANT("Optimierung: %zu Vergleiche übersprungen (%.1f%% der möglichen Vergleiche)\n", skipped_comparisons, (float)((100.0 * skipped_comparisons) / (range_size * (range_size - 1) / 2)));
        //}
        
        // Keine lokalen Match-Counts mehr zu befreien, nur noch gemeinsamer atomarer Array
        
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
        // Anzahl der Partitionen bestimmen
        size_t num_partitions = input->size;
        
        // Buffer für die Ergebnisse aller Partitionen erstellen
        matching* matching_buffer = new matching[num_partitions];
        
        // Jede Partition sequentiell verarbeiten
        for(size_t p = 0; p < num_partitions; p++)
        {
            partition* part = &input->data[p];
            size_t partition_size = part->size;
            
            // Keine Matches möglich bei zu kleinen Partitionen
            if (partition_size <= 1) {
                matching_buffer[p].size = 0;
                matching_buffer[p].matches = nullptr;
                continue;
            }
            
            // Bei sehr kleinen Partitionen oder wenn nur 1 Thread gewünscht ist,
            // direkt ohne Thread-Verwaltung verarbeiten
            if (partition_size < 50 || numThreads == 1) {
                // Erstelle einen atomaren Match-Counts-Array auch für die Single-Thread-Version
                std::atomic<size_t>* single_thread_match_counts = new std::atomic<size_t>[partition_size]();
                
                match_blocker_intern(part, 0, partition_size, &matching_buffer[p], single_thread_match_counts);
                
                // Freigabe des atomaren Arrays
                delete[] single_thread_match_counts;
                
                DEBUG_MATCH("Partition %zu/%zu verarbeitet: %zu Matches gefunden\n", p+1, num_partitions, matching_buffer[p].size);
                continue;
            }
                        
            // Gemeinsamer, thread-sicherer Match-Counts-Array für alle Threads
            std::atomic<size_t>* shared_match_counts = new std::atomic<size_t>[partition_size]();
            
            // Array zur Verfolgung von Elementen mit vielen Matches (für Debug-Zwecke)
            std::atomic<bool>* debug_track_high_matches = new std::atomic<bool>[partition_size]();
            
            size_t total_comparisons = (partition_size * (partition_size - 1)) / 2;
            size_t comparisons_per_thread = total_comparisons / numThreads;
            
            // Array für Thread-Positionen erstellen
            size_t* offsets = new size_t[numThreads + 1];
            offsets[0] = 0;  // Start bei 0
            offsets[numThreads] = partition_size;  // Ende bei partition_size
            
            // Berechne optimale Schnitte für gleichmäßige Thread-Auslastung (quadratisch)
            size_t completed_comps = 0;
            for (size_t t = 1; t < numThreads; t++) {
                // Finde Position, bei der die Anzahl der Vergleiche ungefähr gleich ist
                size_t i = offsets[t-1];
                
                while (i < partition_size - 1) 
                {
                    size_t comps_in_row = partition_size - i - 1;
                    if (completed_comps + comps_in_row >= comparisons_per_thread * t) 
                    {
                        // Finde optimale Position in dieser Zeile
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
            std::thread** threads = new std::thread*[numThreads];
            matching* thread_results = new matching[numThreads];
            
            // Starte die Threads für die jeweiligen Bereiche
            for (size_t t = 0; t < numThreads; t++) 
            {
                size_t start = offsets[t];
                size_t end = offsets[t+1];
                
                if (start < end) 
                {
                    threads[t] = new std::thread([this, part, start, end, &thread_results, t, numThreads, shared_match_counts]()
                    {
                        // Thread-ID für Debug-Ausgabe
                        DEBUG_MATCH_DETAILED("Thread %zu/%zu gestartet: Bereich [%zu-%zu]\n", t+1, numThreads, start, end);
                        
                        // Ausführung der eigentlichen Matching-Funktion mit gemeinsamen Match-Counts
                        match_blocker_intern(part, start, end, &thread_results[t], shared_match_counts);
                        
                        DEBUG_MATCH_DETAILED("Thread %zu/%zu beendet: %zu Matches gefunden\n", t+1, numThreads, thread_results[t].size);
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
                }
            }
            delete[] threads;
            
            // Ergebnisse zusammenführen
            size_t total_matches = 0;
            for (size_t t = 0; t < numThreads; t++) {
                total_matches += thread_results[t].size;
            }
            
            // Ergebnispuffer für die Partition allozieren und Matches zusammenführen
            matching_buffer[p].size = total_matches;
            if (total_matches > 0) {
                matching_buffer[p].matches = new match[total_matches];
                if (!matching_buffer[p].matches) {
                    fprintf(stderr, "Error: Memory allocation failed in identify_matches\n");
                    delete[] offsets;
                    delete[] thread_results;
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
            
            DEBUG_MATCH("Partition %zu/%zu verarbeitet: %zu Matches gefunden (mit %zu Threads)\n", 
                   p+1, num_partitions, matching_buffer[p].size, numThreads);
            
            // Statistik über die Verteilung der Matches ausgeben
            size_t max_matches = 0;
            size_t elements_at_limit = 0;
            size_t histogram[MAX_MATCHES_PER_ITEM+1] = {0}; // Histogramm der Match-Anzahlen
            
            for (size_t i = 0; i < partition_size; i++) {
                size_t count = shared_match_counts[i].load(std::memory_order_relaxed);
                max_matches = std::max(max_matches, count);
                if (count == MAX_MATCHES_PER_ITEM) {
                    elements_at_limit++;
                }
                
                // Histogramm aktualisieren
                if (count <= MAX_MATCHES_PER_ITEM) {
                    histogram[count]++;
                }
                
                // Elemente mit sehr vielen Matches identifizieren (mehr als erwartet)
                if (count > MAX_MATCHES_PER_ITEM) {
                    DEBUG_MATCH_IMPORTANT("Element %zu hat %zu Matches (über dem Limit von %d)\n", 
                                       i, count, MAX_MATCHES_PER_ITEM);
                }
            }
            
            // Ausgabe der Statistik
            DEBUG_MATCH("Match-Statistik für Partition %zu:\n", p+1);
            DEBUG_MATCH("- Max Matches pro Element: %zu\n", max_matches);
            DEBUG_MATCH("- Elemente am Limit (%d): %zu von %zu (%.1f%%)\n", 
                       MAX_MATCHES_PER_ITEM, elements_at_limit, partition_size, 
                       100.0 * elements_at_limit / partition_size);
            
            // Histogramm ausgeben
            for (size_t i = 0; i <= MAX_MATCHES_PER_ITEM; i++) {
                DEBUG_MATCH("  - Elemente mit %zu Matches: %zu (%.1f%%)\n", 
                          i, histogram[i], 100.0 * histogram[i] / partition_size);
            }
            
            // Aufräumen
            delete[] offsets;
            delete[] thread_results;
            delete[] shared_match_counts;
            delete[] debug_track_high_matches;
        }
        
        // Ergebnisse in dataSet verpacken
        dataSet<matching>* result = new dataSet<matching>();
        result->size = num_partitions;
        result->data = matching_buffer;
        
        return result;
    }
};
#endif