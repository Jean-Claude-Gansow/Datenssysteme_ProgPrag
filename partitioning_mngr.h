#ifndef PARTITIONING_MNGR_H
#define PARTITIONING_MNGR_H

#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include "DataTypes.h"
#include "Tokenization_mngr.h"


template <typename base_type, typename InType, size_t N>
class Partitioning_mngr 
{
public:
    using Tokenizer = Tokenization_mngr<N, base_type, InType>;

    // Konfigurationsparameter
    struct Config {
        size_t size_threshold = 15000;      // Maximale Partitionsgröße
        bool verbose_logging = false;      // Detaillierte Ausgaben aktivieren
        double overlap_ratio = 0.2;        // 50% Überlappung bei finaler Aufteilung
        category filter_category = assembler_brand;      // Kategorie für die Partitionierung
    };

    // Konstruktor
    explicit Partitioning_mngr(const Config& config = Config()) : config(config) {
        printf("Partitioning_mngr initialisiert mit Schwellwert %zu, Filter-Kategorie %d\n", 
               config.size_threshold, config.filter_category);
    }

    /**
     * @brief Erstellt einfache überlappende Partitionen aus einem Datensatz basierend auf einer Kategorie
     */
    dataSet<partition_t>* create_partitions(
        dataSet<InType>* input_data, 
        Tokenizer* tokenizer, 
        const std::vector<category>& categories) 
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        printf("🔍 Vereinfachte Partitionierung: Beginne mit %zu Einträgen...\n", input_data->size);
        
        if (!input_data || input_data->size == 0) {
            printf("ℹ️ Leerer Eingabedatensatz, erstelle leeres Partitions-Set\n");
            return createEmptyPartitionSet();
        }
        
        // Falls die Daten klein genug sind, erstelle eine einzelne Partition
        if (input_data->size <= config.size_threshold) {
            printf("🔍 Datensatz ist klein genug für eine Partition\n");
            return createSinglePartition(input_data);
        }

        // Wähle die Filterkategorie aus der Config oder nutze die erste Kategorie aus der Liste
        category filter_cat = config.filter_category;
        if (filter_cat < 0 && !categories.empty()) {
            filter_cat = categories[0];
        }
        
        printf("🔍 Filtere nach Kategorie %d für die Partitionierung\n", filter_cat);
        
        // Bestimme die Anzahl der möglichen Token-Werte für diese Kategorie
        size_t num_tokens = tokenizer->m_class_tokens_found[filter_cat];
        printf("   🔍 Kategorie %d hat %zu verschiedene Token-Werte\n", filter_cat, num_tokens);
        
        // Gruppiere Einträge nach Token-Wert
        std::unordered_map<token, std::vector<std::pair<uintptr_t, InType*>>> token_groups;
        std::vector<std::pair<uintptr_t, InType*>> no_token_entries;
        
        // Gruppiere alle Einträge nach ihrem Token-Wert
        for (size_t i = 0; i < input_data->size; i++) {
            uintptr_t id = static_cast<uintptr_t>(i);
            InType* entry = &input_data->data[i];
            token tok = (*entry)[filter_cat];
            
            if (tok == 0) {
                // Eintrag ohne Token-Information
                no_token_entries.push_back({id, entry});
            } else {
                // Eintrag mit Token-Information
                token_groups[tok].push_back({id, entry});
            }
        }
        
        printf("   🔍 %zu Einträge gruppiert: %zu Gruppen mit Token-Info und %zu Einträge ohne Token\n", 
               input_data->size, token_groups.size(), no_token_entries.size());
        
        // Vorbereitung für das Ergebnis
        std::vector<partition_t> result_partitions;
        
        // 1. Erstelle vollständige Einträge mit Token-Info und kopiere die ohne Token-Info in jede Gruppe
        std::vector<std::vector<std::pair<uintptr_t, InType*>>> complete_groups;
        
        // Verarbeite jede Token-Gruppe und füge Einträge ohne Token hinzu
        for (const auto& group : token_groups) {
            const auto& entries = group.second;
            token tok = group.first;
            
            if (entries.empty()) continue;
            
            // Erstelle eine neue Gruppe mit Token-Einträgen und ohne Token-Einträgen
            std::vector<std::pair<uintptr_t, InType*>> complete_group;
            complete_group.reserve(entries.size() + no_token_entries.size());
            
            // Füge erst die Token-Einträge hinzu
            complete_group.insert(complete_group.end(), entries.begin(), entries.end());
            
            // Dann die Einträge ohne Token
            complete_group.insert(complete_group.end(), no_token_entries.begin(), no_token_entries.end());
            
            printf("   🔍 Token %d: %zu Einträge mit Token + %zu ohne Token = %zu Gesamt\n", 
                   tok, entries.size(), no_token_entries.size(), complete_group.size());
                   
            complete_groups.push_back(std::move(complete_group));
        }
        
        // 2. Verarbeite jede vollständige Gruppe und erstelle überlappende Partitionen wenn nötig
        for (size_t g = 0; g < complete_groups.size(); g++) {
            const auto& group = complete_groups[g];
            
            printf("   🔍 Verarbeite Gruppe %zu mit %zu Einträgen\n", g, group.size());
            
            if (group.size() <= config.size_threshold) {
                // Wenn die Gruppe klein genug ist, erstelle eine einzelne Partition
                partition_t part;
                part.size = group.size();
                part.capacity = group.size();
                part.data = new pair[group.size()]();
                
                for (size_t i = 0; i < group.size(); i++) {
                    part.data[i][0] = group[i].first;
                    part.data[i][1] = reinterpret_cast<uintptr_t>(group[i].second);
                }
                
                result_partitions.push_back(part);
                printf("      ✓ Gruppe %zu: Einzelne Partition mit %zu Einträgen erstellt\n", g, group.size());
            } else {
                // Erstelle überlappende Partitionen für diese vollständige Gruppe
                createOverlappingPartitionsForGroup(group, result_partitions);
            }
        }
        
        // Erstelle das Ergebnisdatenset
        dataSet<partition_t>* result = new dataSet<partition_t>();
        result->size = result_partitions.size();
        result->data = new partition_t[result_partitions.size()];
        
        for (size_t i = 0; i < result_partitions.size(); i++) {
            result->data[i] = result_partitions[i];
        }
        
        // End-Zeit erfassen und Dauer berechnen
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        
        printf("✅ Vereinfachte Partitionierung abgeschlossen: %zu Partitionen erstellt (%.2f Sekunden)\n", 
               result->size, duration / 1000.0);
        
        // Statistiken ausgeben
        if (result->size > 0) {
            size_t min_size = result->data[0].size;
            size_t max_size = 0;
            size_t total_size = 0;
            
            for (size_t i = 0; i < result->size; i++) {
                size_t size = result->data[i].size;
                min_size = std::min(min_size, size);
                max_size = std::max(max_size, size);
                total_size += size;
            }
            
            double avg_size = (result->size > 0) ? (double)total_size / result->size : 0;
            printf("📊 Statistik: Min=%zu, Max=%zu, Durchschnitt=%.1f Einträge pro Partition\n", 
                   min_size, max_size, avg_size);
        }
        
        // Wenn es keine Token-Gruppen gibt, aber Einträge ohne Token-Information
        if (result_partitions.empty() && !no_token_entries.empty()) 
        {
            handleOnlyNoTokenEntries(no_token_entries, result_partitions);
        }
        
        return result;
    }

private:
    Config config;
    
    /**
     * @brief Erstellt überlappende Teilpartitionen für eine Gruppe von Einträgen
     */
    void createOverlappingPartitionsForGroup(
        const std::vector<std::pair<uintptr_t, InType*>>& entries, 
        std::vector<partition_t>& result_partitions) 
    {
        size_t size = entries.size();
        double overlap = config.overlap_ratio;
        size_t target_size = config.size_threshold;
        
        // Berechne Schritt-Größe basierend auf der Überlappung
        size_t step_size = static_cast<size_t>(target_size * (1.0 - overlap));
        if (step_size < 1) step_size = 1;
        
        // Berechne die Anzahl der benötigten Partitionen
        size_t num_parts = (size + step_size - 1) / step_size;
        if (num_parts == 1) num_parts = 1;
        
        printf("      🔍 Erstelle %zu überlappende Partitionen (Überlappung: %.0f%%) für %zu Einträge\n", 
               num_parts, overlap * 100, size);
        
        for (size_t part = 0; part < num_parts; part++) {
            // Berechne Start- und Endindex für diese Partition
            size_t start_idx = part * step_size;
            size_t end_idx = start_idx + target_size;
            if (end_idx > size) end_idx = size;
            
            size_t part_size = end_idx - start_idx;
            
            // Erstelle eine neue Partition
            partition_t part_obj;
            part_obj.size = part_size;
            part_obj.capacity = part_size;
            part_obj.data = new pair[part_size]();
            
            for (size_t i = 0; i < part_size; i++) {
                size_t src_idx = start_idx + i;
                part_obj.data[i][0] = entries[src_idx].first;
                part_obj.data[i][1] = reinterpret_cast<uintptr_t>(entries[src_idx].second);
            }
            
            result_partitions.push_back(part_obj);
            printf("         ✓ Teilpartition %zu/%zu: %zu Einträge (%.1f%% des Schwellwerts)\n", part + 1, num_parts, part_size, (part_size * 100.0) / target_size);
            
            if (end_idx >= size) break;
        }
    }
    
    /**
     * @brief Hilfsmethode für die spezielle Behandlung, falls es nur Einträge ohne Token-Information gibt
     */
    void handleOnlyNoTokenEntries(
        const std::vector<std::pair<uintptr_t, InType*>>& no_token_entries,
        std::vector<partition_t>& result_partitions)
    {
        if (no_token_entries.empty()) return;
        
        printf("   🔍 Nur Einträge ohne Token-Information vorhanden (%zu Einträge)\n", 
               no_token_entries.size());
        
        // Erstelle überlappende Partitionen direkt aus diesen Einträgen
        if (no_token_entries.size() <= config.size_threshold) {
            // Wenn klein genug, erstelle eine einzige Partition
            partition_t part;
            part.size = no_token_entries.size();
            part.capacity = no_token_entries.size();
            part.data = new pair[no_token_entries.size()]();
            
            for (size_t i = 0; i < no_token_entries.size(); i++) {
                part.data[i][0] = no_token_entries[i].first;
                part.data[i][1] = reinterpret_cast<uintptr_t>(no_token_entries[i].second);
            }
            
            result_partitions.push_back(part);
            printf("      ✓ Einzelne Partition für alle %zu Einträge ohne Token-Info erstellt\n", 
                   no_token_entries.size());
        } else {
            // Sonst erstelle überlappende Partitionen
            createOverlappingPartitionsForGroup(no_token_entries, result_partitions);
        }
    }
    
    /**
     * @brief Erstellt eine einzelne Partition mit allen Einträgen
     */
    dataSet<partition_t>* createSinglePartition(dataSet<InType>* input_data) {
        printf("🔍 Erstelle einzelne Partition für %zu Einträge...\n", input_data->size);
        
        partition_t single_partition;
        single_partition.size = input_data->size;
        single_partition.capacity = input_data->size;
        single_partition.data = new pair[input_data->size]();
        
        for (size_t i = 0; i < input_data->size; i++) {
            single_partition.data[i][0] = static_cast<uintptr_t>(i);
            single_partition.data[i][1] = reinterpret_cast<uintptr_t>(&input_data->data[i]);
        }
        
        dataSet<partition_t>* result = new dataSet<partition_t>();
        result->size = 1;
        result->data = new partition_t[1];
        result->data[0] = single_partition;
        
        printf("✅ Einzelne Partition erstellt.\n");
        return result;
    }
    
    /**
     * @brief Erstellt ein leeres Partitionierungs-Dataset
     */
    dataSet<partition_t>* createEmptyPartitionSet() {
        printf("ℹ️ Erstelle leeres Partitions-Set.\n");
        
        dataSet<partition_t>* result = new dataSet<partition_t>();
        result->size = 0;
        result->data = nullptr;
        return result;
    }
};

#endif // PARTITIONING_MNGR_H