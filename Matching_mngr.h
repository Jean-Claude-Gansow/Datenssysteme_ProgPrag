#ifndef MATCHING_MANAGER
#define MATCHING_MANAGER

#include <stdlib.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <unordered_map>
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
    double jaccard_threshhold = 0.80;

public:
    Matching_mngr(size_t unique_id_count) : unique_id_count(unique_id_count) 
    {    
        jaccard_cache = new std::unordered_set<uint32_t>*[unique_id_count];
        // Alle Einträge auf nullptr setzen
        for (size_t i = 0; i < unique_id_count; ++i) 
        {
            jaccard_cache[i] = nullptr;
        }
    }

    ~Matching_mngr() {
        // Speicher der Sets freigeben
        if (jaccard_cache != nullptr) {
            for (size_t i = 0; i < unique_id_count; ++i) {
                if (jaccard_cache[i] != nullptr) {
                    delete jaccard_cache[i];
                    jaccard_cache[i] = nullptr; // Setze Pointer auf nullptr
                }
            }
            delete[] jaccard_cache;
            jaccard_cache = nullptr; // Setze Pointer auf nullptr
        }
    }

    void prepare_all_jaccard_sets(dataSet<compType> *dataset)
    {
        if (dataset == nullptr || dataset->data == nullptr) {
            fprintf(stderr, "ERROR: Invalid dataset passed to prepare_all_jaccard_sets\n");
            return;
        }

        printf("Preparing Jaccard sets for %zu elements in the complete dataset...\n", dataset->size);

        size_t sets_created = 0;
        size_t errors_encountered = 0;

        // Numerische Buffer auf dem Heap allokieren (statt auf dem Stack)
        constexpr size_t BUFFER_SIZE = 750;
        uint32_t *numeral_buffer = nullptr;

        try {
            numeral_buffer = new uint32_t[BUFFER_SIZE]();
        } catch (const std::bad_alloc& e) {
            fprintf(stderr, "ERROR: Failed to allocate numeral_buffer: %s\n", e.what());
            return;
        }

        // Erstelle Sets für alle Einträge des Datensatzes
        for (size_t i = 0; i < dataset->size; i++)
        {
            if (i >= unique_id_count) {
                fprintf(stderr, "ERROR: ID %zu exceeds unique_id_count (%zu)\n", i, unique_id_count);
                errors_encountered++;
                continue;
            }

            uintptr_t id = i; // ID entspricht dem Index im Datensatz
            compType *entry = &dataset->data[i];

            // Nur wenn noch kein Set existiert
            if (jaccard_cache[id] == nullptr && entry != nullptr)
            {
                try 
                {
                    entry->numeral_buffer = numeral_buffer;
                    jaccard_cache[id] = entry->generate_set();

                    if (jaccard_cache[id] == nullptr) 
                    {
                        fprintf(stderr, "WARNING: generate_set returned nullptr for ID %lu\n", id);
                        jaccard_cache[id] = new std::unordered_set<uint32_t>(); // Leeres Set erstellen
                    }

                    sets_created++;
                } 
                catch (const std::exception& e) 
                {
                    fprintf(stderr, "ERROR: Exception while creating jaccard set for ID %lu: %s\n", id, e.what());
                    errors_encountered++;

                    if (jaccard_cache[id] == nullptr) 
                    {
                        jaccard_cache[id] = new std::unordered_set<uint32_t>(); // Leeres Set erstellen
                    }
                }

                // Debug-Ausgabe mit reduzierter Häufigkeit
                if (i % 5000 == 0 || i == dataset->size - 1)
                {
                    printf("Created set for ID %lu, size: %zu (%zu/%zu) - Total sets: %zu\n", id, jaccard_cache[id] ? jaccard_cache[id]->size() : 0, i + 1, dataset->size, sets_created);
                }
            }
        }

        // Speicher wieder freigeben
        delete[] numeral_buffer;
        numeral_buffer = nullptr; // Setze Pointer auf nullptr

        printf("Finished preparing all Jaccard sets - Created %zu sets total, encountered %zu errors\n", sets_created, errors_encountered);
    }

    // Wrapper-Funktion für den Jaccard-Vergleich
    double jaccard_compare(uintptr_t id_i, compType* entry_i, uintptr_t id_j, compType* entry_j) 
    {
        // Jaccard-Vergleich durchführen
        const std::unordered_set<uint32_t>& set_i = *(jaccard_cache[id_i]);
        const std::unordered_set<uint32_t>& set_j = *(jaccard_cache[id_j]);

        //printf("DEBUG: Final set sizes - set_i: %zu, set_j: %zu\n", set_i.size(), set_j.size());

        // Vermeiden von Division durch Null
        if (set_i.empty() && set_j.empty()) 
        {
            printf("DEBUG: jaccard_compare - Both sets are empty\n");
            return false;
        }

        size_t intersection_size = 0;
        for (const auto& elem : set_i) 
        {
            if (set_j.find(elem) != set_j.end()) 
            {
                intersection_size++;
            }
        }

        size_t union_size = set_i.size() + set_j.size() - intersection_size;
        double jaccard_index = static_cast<double>(intersection_size) / union_size;
        
        return jaccard_index;
    }

    void match_blocker_intern( partition* part, size_t start, size_t end, matching* findingsBuffer)
    {
        // Debug-Ausgaben zur Überwachung der Funktionsaufrufe        
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

        // Maximale Kapazität des Buffers berechnen
        size_t range_size = end - start;
        if (range_size == 0) 
        {
            return;  // Nichts zu tun
        }
        
        size_t max_possible_matches = (range_size * (range_size - 1)) / 2;
        if (max_possible_matches == 0) 
        {
            return;  // Keine möglichen Matches
        }
        
        // Temp-Puffer für die Matches erstellen
        try 
        {
            findingsBuffer->matches = new match[max_possible_matches]();
        } 
        catch (const std::bad_alloc& e) 
        {
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
            
            // Prüfen ob id_i innerhalb der Array-Grenzen liegt
            if (id_i >= unique_id_count) 
            {
                fprintf(stderr, "Error: ID %lu out of bounds (max: %zu)\n", id_i, unique_id_count-1);
                continue;
            }
            
            // Vergleiche mit allen nachfolgenden Einträgen
            for (size_t j = i + 1; j < end; ++j) {
                
                uintptr_t id_j = (uintptr_t)part->data[j][0];
                compType* entry_j = reinterpret_cast<compType*>(part->data[j][1]);

                // Prüfen ob id_j innerhalb der Array-Grenzen liegt
                if (id_j >= unique_id_count) {
                    fprintf(stderr, "Error: ID %lu out of bounds (max: %zu)\n", id_j, unique_id_count-1);
                    continue;
                }
                
                // Prüfen ob die jaccard_cache-Einträge existieren
                if (jaccard_cache[id_i] == nullptr || jaccard_cache[id_j] == nullptr) {
                    fprintf(stderr, "Warning: Missing jaccard_cache for ID %lu or %lu\n", id_i, id_j);
                    continue;
                }

                comparison_count++;
                double jaccar_score = jaccard_compare(id_i, entry_i, id_j, entry_j);
                if (jaccar_score >= jaccard_threshhold)
                {
                    printf("[%lu == %lu]\n", id_i, id_j);
                    findingsBuffer->matches[match_count].data[0] = id_i;
                    findingsBuffer->matches[match_count].data[1] = id_j;
                    findingsBuffer->matches[match_count].jaccard_index = jaccar_score;
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
        
        printf("identify_matches: Verarbeite %zu Partitionen mit %zu Threads\n", input->size, numThreads);
        
        size_t num_partitions = input->size;
        matching* matching_buffer = new matching[num_partitions]();
        
        for (size_t p = 0; p < num_partitions; p++) {
            partition* part = &input->data[p];
            
            printf("Verarbeite Partition %zu: Größe=%zu, Daten=%p\n", p, 
                   (part != nullptr) ? part->size : 0, 
                   (part != nullptr) ? (void*)part->data : nullptr);
            
            // Sicherstellen, dass die Partition gültig ist
            if (part == nullptr || part->data == nullptr || part->size <= 1) {
                printf("Überspringe Partition %zu: Ungültige oder zu kleine Partition\n", p);
                matching_buffer[p].size = 0;
                matching_buffer[p].matches = nullptr;
                continue;
            }
            
            // Für kleine Partitionen oder wenn nur ein Thread gewünscht ist
            if (part->size <= 100 || numThreads == 1) {
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

    void apply_transitivity(dataSet<matching> *matches, double threshold)
    {
        if (!matches || !matches->data)
            return;

        printf("Applying GLOBAL transitivity with threshold %.2f...\n", threshold);
        
        // Debug: Gesamtzahl der Matches
        size_t total_matches = 0;
        size_t matches_above_threshold = 0;
        
        // Erstelle ein globales Mapping von Elementen zu ihren Matches für schnellen Zugriff
        std::unordered_map<uintptr_t, std::vector<std::pair<uintptr_t, double>>> global_element_matches;
        
        // Leicht reduzierter Threshold für das Mapping und die Transitivität
        double mapping_threshold = threshold * 0.9;  // 10% niedriger für mehr potentielle Matches
        
        printf("Step 1: Collecting all matches across all partitions...\n");
        
        // Erste Schleife: Sammle alle Matches aus allen Partitionen in das globale Mapping
        for (size_t i = 0; i < matches->size; i++)
        {
            matching &current_match = matches->data[i];
            if (!current_match.matches || current_match.size == 0)
                continue;
            
            total_matches += current_match.size;
            
            // Zähle Matches über dem Threshold und zeige Beispiele an
            for (size_t j = 0; j < current_match.size; j++) 
            {
                uintptr_t id_a = current_match.matches[j].data[0];
                uintptr_t id_b = current_match.matches[j].data[1];
                double jaccard_index = current_match.matches[j].jaccard_index;
                
                if (jaccard_index >= threshold) {
                    matches_above_threshold++;
                    if (matches_above_threshold <= 5) {  // Zeige max. 5 Beispiele
                        printf("Example high-jaccard match: %lu == %lu (%.3f)\n", 
                              id_a, id_b, jaccard_index);
                    }
                }
                
                // Füge alle Matches in das globale Mapping ein (wir verwenden hier den reduzierten Threshold)
                if (jaccard_index >= mapping_threshold) {
                    global_element_matches[id_a].push_back({id_b, jaccard_index});
                    global_element_matches[id_b].push_back({id_a, jaccard_index}); // Bidirektional speichern
                }
            }
        }
        
        printf("Collected %zu total matches across all partitions\n", total_matches);
        printf("Matches above threshold (%.2f): %zu (%.1f%%)\n", 
               threshold, matches_above_threshold, 
               (total_matches > 0) ? (matches_above_threshold * 100.0 / total_matches) : 0.0);
        
        // Debug: Größe des globalen Mappings
        printf("Global mapping size: %zu unique entries\n", global_element_matches.size());
        size_t total_connections = 0;
        for (const auto &entry : global_element_matches) {
            total_connections += entry.second.size();
        }
        printf("Total connections in global mapping: %zu\n", total_connections);
        
        printf("Step 2: Finding transitive matches across all partitions...\n");
        
        // Sammle alle neuen transitiven Matches in einer globalen Liste
        std::vector<std::tuple<uintptr_t, uintptr_t, double, size_t>> global_transitive_matches;
        // <id_a, id_c, jaccard_index, partition_index>
        
        size_t check_count = 0;
        size_t found_count = 0;
        
        // Suche nach transitiven Beziehungen im globalen Mapping
        for (const auto &elem_entry : global_element_matches)
        {
            uintptr_t id_a = elem_entry.first;
            
            if (elem_entry.second.empty()) {
                continue;  // Überspringe Einträge ohne Verbindungen
            }
            
            // Für jedes Element, mit dem id_a verknüpft ist
            for (const auto &match_a : elem_entry.second)
            {
                uintptr_t id_b = match_a.first;
                double jaccard_ab = match_a.second;
                
                // Für jedes Element, mit dem id_b verknüpft ist
                auto it_b = global_element_matches.find(id_b);
                if (it_b != global_element_matches.end() && !it_b->second.empty())
                {
                    for (const auto &match_b : it_b->second)
                    {
                        uintptr_t id_c = match_b.first;
                        double jaccard_bc = match_b.second;
                        
                        check_count++;
                        
                        // Überprüfe, ob ein transitives Match existieren sollte
                        if (id_c != id_a && jaccard_bc >= mapping_threshold)
                        {
                            // Berechne für welche Partition das Match gilt
                            // Hier: vereinfachte Annahme - füge es zur ersten Partition hinzu,
                            // in der entweder id_a oder id_c vorkommt
                            size_t target_partition = 0; // Default zur ersten Partition
                            
                            // Überprüfe, ob dieses Match bereits existiert (in jeder Partition)
                            bool already_exists = false;
                            
                            // Prüfe in allen bestehenden Matches aller Partitionen
                            for (size_t p = 0; p < matches->size && !already_exists; p++)
                            {
                                matching &part_matches = matches->data[p];
                                if (!part_matches.matches) continue;
                                
                                for (size_t m = 0; m < part_matches.size; m++)
                                {
                                    if ((part_matches.matches[m].data[0] == id_a &&
                                         part_matches.matches[m].data[1] == id_c) ||
                                        (part_matches.matches[m].data[0] == id_c &&
                                         part_matches.matches[m].data[1] == id_a))
                                    {
                                        already_exists = true;
                                        break;
                                    }
                                }
                            }
                            
                            // Prüfe auch gegen die bereits gefundenen transitiven Matches
                            if (!already_exists)
                            {
                                for (const auto &tm : global_transitive_matches)
                                {
                                    uintptr_t tm_a = std::get<0>(tm);
                                    uintptr_t tm_c = std::get<1>(tm);
                                    
                                    if ((tm_a == id_a && tm_c == id_c) || (tm_a == id_c && tm_c == id_a))
                                    {
                                        already_exists = true;
                                        break;
                                    }
                                }
                            }
                            
                            // Wenn nicht bereits vorhanden, füge neues transitives Match hinzu
                            if (!already_exists)
                            {
                                // Berechne den Jaccard-Index für das transitive Match
                                double new_jaccard = std::min(jaccard_ab, jaccard_bc) * 0.95; // Leicht reduzierter Jaccard-Index
                                
                                // Bestimme die Partition, zu der das Match gehört
                                // Suche nach der ersten Partition, die id_a oder id_c enthält
                                for (size_t p = 0; p < matches->size; p++)
                                {
                                    matching &part_matches = matches->data[p];
                                    if (!part_matches.matches) continue;
                                    
                                    bool found_in_partition = false;
                                    for (size_t m = 0; m < part_matches.size; m++)
                                    {
                                        if (part_matches.matches[m].data[0] == id_a || 
                                            part_matches.matches[m].data[1] == id_a ||
                                            part_matches.matches[m].data[0] == id_c || 
                                            part_matches.matches[m].data[1] == id_c)
                                        {
                                            target_partition = p;
                                            found_in_partition = true;
                                            break;
                                        }
                                    }
                                    if (found_in_partition) break;
                                }
                                
                                // Füge zum globalen transitiven Match-Array hinzu
                                global_transitive_matches.push_back(std::make_tuple(id_a, id_c, new_jaccard, target_partition));
                                found_count++;
                                
                                if (found_count <= 10) {  // Zeige die ersten 10 transitiven Matches
                                    printf("New global transitive match: %lu -> %lu -> %lu (%.3f, %.3f -> %.3f) [Partition %zu]\n", 
                                           id_a, id_b, id_c, jaccard_ab, jaccard_bc, new_jaccard, target_partition);
                                }
                                
                                if (found_count % 100 == 0) {
                                    printf("  Found %zu global transitive matches so far (checked %zu potential matches)\n",
                                           found_count, check_count);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        printf("Step 3: Adding %zu new transitive matches to partitions...\n", global_transitive_matches.size());
        
        // Zähler für hinzugefügte Matches pro Partition
        std::vector<size_t> matches_per_partition(matches->size, 0);
        
        // Gruppiere die transitiven Matches nach Partitionen
        std::unordered_map<size_t, std::vector<match>> partition_new_matches;
        
        for (const auto &tm : global_transitive_matches)
        {
            uintptr_t id_a = std::get<0>(tm);
            uintptr_t id_c = std::get<1>(tm);
            double jaccard = std::get<2>(tm);
            size_t partition = std::get<3>(tm);
            
            match new_match;
            new_match.data[0] = id_a;
            new_match.data[1] = id_c;
            new_match.jaccard_index = jaccard;
            
            partition_new_matches[partition].push_back(new_match);
        }
        
        // Füge die neuen Matches zu den entsprechenden Partitionen hinzu
        for (const auto &entry : partition_new_matches)
        {
            size_t partition = entry.first;
            const std::vector<match> &new_matches = entry.second;
            
            if (partition >= matches->size) {
                printf("Warning: Invalid partition index %zu, skipping %zu matches\n", 
                       partition, new_matches.size());
                continue;
            }
            
            matching &current_match = matches->data[partition];
            
            // Wenn keine neuen Matches für diese Partition, überspringe
            if (new_matches.empty()) continue;
            
            // Erstelle ein neues größeres Array
            size_t old_size = current_match.size;
            size_t new_size = old_size + new_matches.size();
            
            match *new_array = new match[new_size];
            
            // Kopiere existierende Matches
            if (current_match.matches) {
                memcpy(new_array, current_match.matches, old_size * sizeof(match));
            }
            
            // Kopiere neue transitive Matches
            for (size_t m = 0; m < new_matches.size(); m++)
            {
                new_array[old_size + m] = new_matches[m];
            }
            
            // Ersetze das alte Array
            delete[] current_match.matches;
            current_match.matches = new_array;
            current_match.size = new_size;
            
            matches_per_partition[partition] = new_matches.size();
        }
        
        // Ausgabe der Ergebnisse pro Partition
        size_t total_transitive_matches = 0;
        for (size_t i = 0; i < matches_per_partition.size(); i++) {
            if (matches_per_partition[i] > 0) {
                printf("Partition %zu: Added %zu transitive matches\n", i, matches_per_partition[i]);
                total_transitive_matches += matches_per_partition[i];
            }
        }
        
        printf("Global transitivity application completed: Added %zu transitive matches in total\n", total_transitive_matches);
    }
};
#endif