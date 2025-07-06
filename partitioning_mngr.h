#ifndef PARTITIONING_MNGR_H
#define PARTITIONING_MNGR_H

#include <vector>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include "DataTypes.h"
#include "Tokenization_mngr.h"

// Annahme: InType hat operator[](size_t) für Tokenzugriff
template <typename base_type, typename InType, size_t N>
class Partitioning_mngr 
{
public:
    using Tokenizer = Tokenization_mngr<N, base_type, InType>;

    Partitioning_mngr(size_t threshhold)
    {
        this->size_threshhold = threshhold;
        printf("Setting size threshold for partitioning to %zu\n", this->size_threshhold);
    }

    // Wrapper-Funktion, die das Eingabe-Dataset in ein Dataset von Partitionen umwandelt
    dataSet<partition_t>* create_partitions(dataSet<InType>* input_data, Tokenizer* tokenizer, const std::vector<category>& class_hierarchy) {
        printf("Creating partitions for dataset with %zu elements\n", input_data->size);
        
        // Wenn die Hierarchie leer ist oder die Daten unter dem Threshold liegen
        if (class_hierarchy.empty() || input_data->size <= size_threshhold) 
        {
            // Erstelle eine einzelne Partition mit allen Daten
            partition_t single_partition;
            single_partition.size = input_data->size;
            single_partition.capacity = input_data->size;
            single_partition.data = new pair[input_data->size];
            
            // Fülle die Partition mit Paaren (Index, Pointer)
            for (size_t i = 0; i < input_data->size; i++) 
            {
                single_partition.data[i][0] = (uintptr_t)i;                   // Index
                single_partition.data[i][1] = (uintptr_t)&input_data->data[i]; // Pointer
            }
            
            // Erstelle das Rückgabe-Dataset und initialisiere es mit der einzelnen Partition
            dataSet<partition_t>* result = new dataSet<partition_t>();
            result->size = 1;
            result->data = new partition_t[1];
            result->data[0] = single_partition;
            
            return result;
        }
        
        // Sammlung aller erzeugten Partitionen
        std::vector<partition_t> all_partitions;
        
        // Erstelle zunächst die Trash-Partitionen für Einträge mit wenigen Tokens
        partition_t trash_0, trash_1, trash_2;
        createTrashPartitions(input_data, trash_0, trash_1, trash_2);
        
        // Füge die nicht-leeren Trash-Partitionen zur Sammlung hinzu
        if (trash_0.size > 0) all_partitions.push_back(trash_0);
        if (trash_1.size > 0) all_partitions.push_back(trash_1);
        if (trash_2.size > 0) all_partitions.push_back(trash_2);
        
        // Erstelle die regulären Partitionen für Einträge mit ausreichend Tokens
        std::vector<partition_t> regular_partitions;
        createRegularPartitions(input_data, tokenizer, class_hierarchy, regular_partitions);
        
        // Füge alle regulären Partitionen zur Gesamtsammlung hinzu
        all_partitions.insert(all_partitions.end(), regular_partitions.begin(), regular_partitions.end());
        
        // Erstelle das Rückgabe-Dataset mit allen gültigen Partitionen
        dataSet<partition_t>* result = new dataSet<partition_t>();
        
        // Filtere leere Partitionen und solche mit nur einem Element
        std::vector<partition_t> valid_partitions;
        for (auto& part : all_partitions) {
            if (part.size > 1) {
                valid_partitions.push_back(part);
            } else if (part.size == 1) {
                // Befreie den Speicher für Partitionen mit nur einem Element
                delete[] part.data;
            }
        }
        
        // Erstelle das endgültige Dataset
        result->size = valid_partitions.size();
        result->data = new partition_t[valid_partitions.size()];
        
        // Kopiere alle gültigen Partitionen ins Ergebnis
        for (size_t i = 0; i < valid_partitions.size(); i++) {
            result->data[i] = valid_partitions[i];
        }
        
        printf("Final partition count: %zu\n", result->size);
        return result;
    }

private:
    // Hilfsfunktion, um leserlichen Namen für eine Kategorie zurückzugeben
    const char* getCategoryName(category cat) {
        static const char* names[] = {
            "assembler_brand", "assembler_modell", "ram_capacity/storage_capacity", 
            "rom_capacity/class_a", "cpu_brand/class_c", "cpu_fam/class_v",
            "cpu_series/class_u", "gpu_brand/class_uhs", "gpu_fam/variant",
            "gpu_series/data_speed", "display_resolution/formfactor", "display_size/connection_type"
        };
        
        if (cat >= 0 && cat < 12) {
            return names[cat];
        }
        return "unknown";
    }
    
    // Erstellt die drei Trash-Partitionen für Einträge mit 0, 1 oder 2 Tokens
    void createTrashPartitions(dataSet<InType>* input_data, 
                              partition_t& trash_0, partition_t& trash_1, partition_t& trash_2) {
        printf("Creating trash partitions for entries with ≤2 tokens\n");
        
        // Initialisiere die Trash-Partitionen
        trash_0.size = 0;
        trash_0.capacity = input_data->size;
        trash_0.data = new pair[input_data->size];
        
        trash_1.size = 0;
        trash_1.capacity = input_data->size;
        trash_1.data = new pair[input_data->size];
        
        trash_2.size = 0;
        trash_2.capacity = input_data->size;
        trash_2.data = new pair[input_data->size];
        
        // Zähle, wie viele Einträge in jede Trash-Partition kommen
        size_t count_0 = 0, count_1 = 0, count_2 = 0;
        
        // Verteile die Einträge auf die Trash-Partitionen basierend auf token_count
        for (size_t i = 0; i < input_data->size; i++) {
            InType* entry = &input_data->data[i];
            
            if (entry->token_count == 0) {
                trash_0.data[trash_0.size][0] = (uintptr_t)i;      // Index
                trash_0.data[trash_0.size][1] = (uintptr_t)entry;  // Pointer
                trash_0.size++;
                count_0++;
            } 
            else if (entry->token_count == 1) {
                trash_1.data[trash_1.size][0] = (uintptr_t)i;      // Index
                trash_1.data[trash_1.size][1] = (uintptr_t)entry;  // Pointer
                trash_1.size++;
                count_1++;
            }
            else if (entry->token_count == 2) {
                trash_2.data[trash_2.size][0] = (uintptr_t)i;      // Index
                trash_2.data[trash_2.size][1] = (uintptr_t)entry;  // Pointer
                trash_2.size++;
                count_2++;
            }
        }
        
        // Gib Informationen über die erstellten Trash-Partitionen aus
        printf("Trash partitions created:\n");
        printf("  - Trash 0 (0 tokens): %zu entries\n", count_0);
        printf("  - Trash 1 (1 token):  %zu entries\n", count_1);
        printf("  - Trash 2 (2 tokens): %zu entries\n", count_2);
        printf("  - Total trash entries: %zu (%.1f%% of dataset)\n", 
               count_0 + count_1 + count_2, 
               100.0 * (count_0 + count_1 + count_2) / input_data->size);
               
        // Befreie nicht benötigte Trash-Partitionen
        if (count_0 == 0) {
            delete[] trash_0.data;
            trash_0.data = nullptr;
        }
        
        if (count_1 == 0) {
            delete[] trash_1.data;
            trash_1.data = nullptr;
        }
        
        if (count_2 == 0) {
            delete[] trash_2.data;
            trash_2.data = nullptr;
        }
    }
    
    // Erstellt und verarbeitet die regulären Partitionen rekursiv
    void createRegularPartitions(dataSet<InType>* input_data, Tokenizer* tokenizer, 
                                const std::vector<category>& class_hierarchy,
                                std::vector<partition_t>& output_partitions) {
        printf("\nCreating regular partitions for entries with >2 tokens\n");
        
        // Erstelle eine anfängliche Partition mit allen Einträgen mit 3+ Tokens
        partition_t initial_partition;
        initial_partition.size = 0;
        initial_partition.capacity = input_data->size;
        initial_partition.data = new pair[input_data->size];
        
        // Zähle Einträge mit ausreichend Tokens
        size_t valid_entries = 0;
        
        for (size_t i = 0; i < input_data->size; i++) {
            InType* entry = &input_data->data[i];
            
            if (entry->token_count > 2) {
                initial_partition.data[initial_partition.size][0] = (uintptr_t)i;      // Index
                initial_partition.data[initial_partition.size][1] = (uintptr_t)entry;  // Pointer
                initial_partition.size++;
                valid_entries++;
            }
        }
        
        printf("Initial regular partition created with %zu entries (%.1f%% of dataset)\n",
               valid_entries, 100.0 * valid_entries / input_data->size);
               
        if (valid_entries == 0) {
            // Keine gültigen Einträge, befreie den Speicher und beende
            delete[] initial_partition.data;
            return;
        }
        
        // Beginne mit der rekursiven Partitionierung, falls die Anzahl der Einträge über dem Threshold liegt
        if (valid_entries > size_threshhold) {
            // Vector zum Sammeln der Partitionen
            std::vector<partition_t> partitions;
            partitions.push_back(initial_partition);
            
            // Rekursive Partitionierung durch alle Hierarchieebenen
            partitionRecursively(partitions, tokenizer, class_hierarchy, 0);
            
            // Füge alle erzeugten Partitionen zur Ausgabe hinzu
            for (auto& part : partitions) {
                if (part.size > 0) {
                    output_partitions.push_back(part);
                } else {
                    delete[] part.data;
                }
            }
        } else {
            // Wenn die anfängliche Partition klein genug ist, füge sie direkt hinzu
            output_partitions.push_back(initial_partition);
        }
    }
    
    // Rekursive Partitionierung durch die Hierarchieebenen
    void partitionRecursively(std::vector<partition_t>& partitions, Tokenizer* tokenizer,
                             const std::vector<category>& class_hierarchy, size_t level) {
        // Wenn alle Hierarchieebenen verarbeitet wurden, beende die Rekursion
        if (level >= class_hierarchy.size()) {
            return;
        }
        
        // Bestimme die aktuelle Kategorie für die Partitionierung
        category current_category = class_hierarchy[level];
        printf("\nPartitioning at level %zu using category %s\n", 
               level + 1, getCategoryName(current_category));
        
        // Bestimme die Anzahl der Tokens für die aktuelle Kategorie
        size_t num_tokens = tokenizer->m_class_tokens_found[current_category];
        
        if (num_tokens == 0) {
            printf("Warning: No tokens found for category %s, skipping to next level\n", 
                  getCategoryName(current_category));
            partitionRecursively(partitions, tokenizer, class_hierarchy, level + 1);
            return;
        }
        
        printf("Found %zu tokens for category %s\n", num_tokens, getCategoryName(current_category));
        
        // Verarbeite jede bestehende Partition
        size_t original_count = partitions.size();
        
        for (size_t p_idx = 0; p_idx < original_count; p_idx++) {
            partition_t& current = partitions[p_idx];
            
            // Wenn die Partition leer ist oder zu klein, überspringe sie
            if (current.size <= 1 || current.size <= size_threshhold) {
                printf("Skipping partition %zu (size: %zu, threshold: %zu)\n", 
                      p_idx, current.size, size_threshhold);
                continue;
            }
            
            printf("Processing partition %zu with %zu entries\n", p_idx, current.size);
            
            // Erstelle Subpartitionen für jeden Token-Wert
            std::vector<partition_t> subparts(num_tokens);
            for (size_t t = 0; t < num_tokens; t++) {
                subparts[t].size = 0;
                subparts[t].capacity = current.size;
                subparts[t].data = new pair[current.size];
            }
            
            // Zähler für statistische Informationen
            size_t entries_with_info = 0;
            size_t entries_without_info = 0;
            
            // Verteile die Einträge auf die Subpartitionen
            for (size_t i = 0; i < current.size; i++) {
                InType* entry = (InType*)current.data[i][1];
                uintptr_t entry_id = current.data[i][0];
                
                // Hole den Token-Wert für die aktuelle Kategorie
                token tok_value = (*entry)[current_category];
                
                if (tok_value == 0) {
                    // Keine Information für diese Kategorie, kopiere in alle Subpartitionen
                    entries_without_info++;
                    for (size_t t = 0; t < num_tokens; t++) {
                        subparts[t].data[subparts[t].size][0] = entry_id;
                        subparts[t].data[subparts[t].size][1] = (uintptr_t)entry;
                        subparts[t].size++;
                    }
                } 
                else if (tok_value <= num_tokens) {
                    // Spezifische Information vorhanden, füge zur entsprechenden Subpartition hinzu
                    entries_with_info++;
                    size_t t_idx = tok_value - 1;  // Token-Werte beginnen bei 1
                    subparts[t_idx].data[subparts[t_idx].size][0] = entry_id;
                    subparts[t_idx].data[subparts[t_idx].size][1] = (uintptr_t)entry;
                    subparts[t_idx].size++;
                }
                else {
                    // Token-Wert außerhalb des gültigen Bereichs
                    printf("Warning: Token value %hu for category %s exceeds valid range (1-%zu)\n",
                           tok_value, getCategoryName(current_category), num_tokens);
                }
            }
            
            printf("Partition %zu distribution: %zu with info, %zu without info\n",
                   p_idx, entries_with_info, entries_without_info);
                   
            // Gib detaillierte Informationen für jede Subpartition aus
            printf("Subpartition sizes:\n");
            for (size_t t = 0; t < num_tokens; t++) {
                printf("  - Subpart %zu (token %zu): %zu entries\n", 
                       t, t+1, subparts[t].size);
            }
            
            // Entferne die aktuelle Partition und ersetze sie durch ihre Subpartitionen
            delete[] current.data;
            current.data = nullptr;
            current.size = 0;
            
            // Füge alle nicht-leeren Subpartitionen hinzu
            for (auto& subpart : subparts) {
                if (subpart.size > 0) {
                    partitions.push_back(std::move(subpart));
                } else {
                    delete[] subpart.data;
                }
            }
        }
        
        // Entferne alle leeren Partitionen (die durch die obige Schleife entstanden sind)
        partitions.erase(
            std::remove_if(partitions.begin(), partitions.end(), 
                [](const partition_t& p) { return p.size == 0; }),
            partitions.end()
        );
        
        // Gib eine Zusammenfassung der aktuellen Partitionen aus
        size_t total_entries = 0;
        size_t partitions_above_threshold = 0;
        
        for (const auto& part : partitions) {
            total_entries += part.size;
            if (part.size > size_threshhold) {
                partitions_above_threshold++;
            }
        }
        
        printf("\nAfter level %zu (%s):\n", level + 1, getCategoryName(current_category));
        printf("  - Total partitions: %zu\n", partitions.size());
        printf("  - Total entries: %zu\n", total_entries);
        printf("  - Partitions above threshold: %zu\n", partitions_above_threshold);
        
        // Wenn nötig, fahre mit der nächsten Hierarchieebene fort
        if (partitions_above_threshold > 0 && level + 1 < class_hierarchy.size()) {
            partitionRecursively(partitions, tokenizer, class_hierarchy, level + 1);
        } else if (partitions_above_threshold > 0) {
            printf("WARNING: Still have partitions above threshold but no more categories\n");
        } else {
            printf("All partitions are now below threshold, stopping recursion\n");
        }
    }

private:
    size_t size_threshhold;
};

#endif // PARTITIONING_MNGR_H