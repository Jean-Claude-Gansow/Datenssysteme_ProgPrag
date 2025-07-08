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
        double overlap_ratio = 0.2;        // Überlappungsfaktor
        category filter_category = assembler_brand;      // Kategorie für die Partitionierung
    };

    // Neuer Konstruktor: Partitionierungsgröße und Overlap-Faktor als Parameter
    Partitioning_mngr(size_t partition_size, double overlap_ratio, bool verbose = false, category filter_cat = assembler_brand)
    {
        config.size_threshold = partition_size;
        config.overlap_ratio = overlap_ratio;
        config.verbose_logging = verbose;
        config.filter_category = filter_cat;
        printf("Partitioning_mngr initialisiert mit Schwellwert %zu, Overlap %.2f, Filter-Kategorie %d\n", config.size_threshold, config.overlap_ratio, config.filter_category);
    }

    // Alternativer Konstruktor für Kompatibilität
    explicit Partitioning_mngr(const Config& config = Config()) : config(config) 
    {
        printf("Partitioning_mngr initialisiert mit Schwellwert %zu, Filter-Kategorie %d\n", config.size_threshold, config.filter_category);
    }

    /**
     * @brief Erstellt hierarchische Partitionen basierend auf mehreren Token-Kategorien
     * @details Teilt die Daten hierarchisch nach jeder Kategorie auf. Unbekannte Tokens landen 
     * in einer eigenen Subpartition, die dann weiter nach dem nächsten Token aufgeteilt wird.
     */
    dataSet<partition_t>* create_partitions(
        dataSet<InType>* input_data, 
        Tokenizer* tokenizer, 
        const std::vector<category>& categories) 
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        printf("🔍 Hierarchische Partitionierung: Beginne mit %zu Einträgen...\n", input_data->size);
        
        if (!input_data || input_data->size == 0) {
            printf("ℹ️ Leerer Eingabedatensatz, erstelle leeres Partitions-Set\n");
            return createEmptyPartitionSet();
        }
        
        // Falls die Daten klein genug sind, erstelle eine einzelne Partition
        if (input_data->size <= config.size_threshold) {
            printf("🔍 Datensatz ist klein genug für eine Partition\n");
            return createSinglePartition(input_data);
        }

        // Verwende die übergebenen Kategorien oder Fallback auf die Konfiguration
        std::vector<category> used_categories;
        if (!categories.empty()) {
            used_categories = categories;
        } else {
            used_categories.push_back(config.filter_category);
        }
        
        printf("🔍 Hierarchische Partitionierung mit %zu Kategorien\n", used_categories.size());
        
        // Erstelle einen Vektor mit allen Einträgen aus dem Input-Datensatz
        std::vector<std::pair<uintptr_t, InType*>> all_entries;
        all_entries.reserve(input_data->size);
        for (size_t i = 0; i < input_data->size; i++) {
            all_entries.emplace_back(static_cast<uintptr_t>(i), &input_data->data[i]);
        }
        
        // Ergebnis-Partitionen
        std::vector<partition_t> result_partitions;
        
        // Starte die rekursive hierarchische Partitionierung
        partitionHierarchically(all_entries, used_categories, 0, tokenizer, result_partitions);
        
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
        
        printf("✅ Hierarchische Partitionierung abgeschlossen: %zu Partitionen erstellt (%.2f Sekunden)\n", 
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
    
    /**
     * @brief Rekursive Methode zur hierarchischen Partitionierung nach Token-Kategorien
     */
    void partitionHierarchically(
        const std::vector<std::pair<uintptr_t, InType*>>& entries,
        const std::vector<category>& categories,
        size_t category_index,
        Tokenizer* tokenizer,
        std::vector<partition_t>& result_partitions)
    {
        // Abbruchbedingung: Wenn keine weiteren Kategorien verfügbar sind oder die Gruppe klein genug
        if (category_index >= categories.size() || entries.size() <= config.size_threshold) {
            if (entries.size() > 0) {
                printf("   🔍 Erzeuge Endpartition mit %zu Einträgen (Kategorie-Index: %zu)\n", 
                       entries.size(), category_index);
                
                // Erstelle überlappende Partitionen für diese Gruppe
                createOverlappingPartitionsForGroup(entries, result_partitions);
            }
            return;
        }
        
        // Aktuelle Kategorie für die Partitionierung
        category current_cat = categories[category_index];
        
        printf("   🔍 Partitionierung nach Kategorie %d (Level %zu) für %zu Einträge\n", 
               current_cat, category_index, entries.size());
        
        // Map zur Gruppierung von Einträgen nach Token-Wert
        std::unordered_map<token, std::vector<std::pair<uintptr_t, InType*>>> token_groups;
        // Spezielle Gruppe für "unbekannt" (Token = 0)
        std::vector<std::pair<uintptr_t, InType*>> unknown_entries;
        
        // Gruppiere die Einträge nach dem Token-Wert für die aktuelle Kategorie
        for (const auto& entry : entries) {
            // Direkter Zugriff auf den Token-Wert über den Operator []
            // Da current_cat ein enum ist und direkt als Index verwendet werden kann
            token token_value = 0;
            
            // Sicherheitscheck: Ist die Kategorie im gültigen Bereich?
            if (current_cat >= 0 && current_cat < N) {
                token_value = (*entry.second)[current_cat]; // Verwende den [] Operator der InType-Struktur
            }
            
            if (config.verbose_logging && token_value > 0) {
                printf("        🔍 Eintrag %lu: Token-Wert für Kategorie %d ist %hu\n", 
                       entry.first, current_cat, token_value);
            }
            
            // Wenn Token unbekannt (0), in die unbekannt-Gruppe einsortieren
            if (token_value == 0) {
                unknown_entries.push_back(entry);
            } else {
                // Sonst zur entsprechenden Token-Gruppe hinzufügen
                token_groups[token_value].push_back(entry);
            }
        }
        
        printf("      📊 %zu verschiedene Token-Gruppen gefunden, %zu Einträge ohne Token-Info\n", 
               token_groups.size(), unknown_entries.size());
        
        // Verarbeite jede Token-Gruppe rekursiv mit der nächsten Kategorie
        for (const auto& group_pair : token_groups) {
            const auto& group = group_pair.second;
            
            // Debug-Ausgabe für große Gruppen
            if (group.size() > config.size_threshold / 2) {
                printf("      ⚠️ Große Gruppe mit Token %hu: %zu Einträge (%.1f%% des Schwellwerts)\n", 
                       group_pair.first, group.size(), (group.size() * 100.0) / config.size_threshold);
            }
            
            // Rekursiver Aufruf für die nächste Kategorie
            partitionHierarchically(group, categories, category_index + 1, tokenizer, result_partitions);
        }
        
        // Spezialfall: Verarbeite Einträge ohne Token-Information
        if (!unknown_entries.empty()) {
            printf("      ℹ️ Verarbeite %zu Einträge ohne Token-Info für Kategorie %d\n", 
                   unknown_entries.size(), current_cat);
            
            if (category_index + 1 < categories.size()) {
                // Wenn weitere Kategorien verfügbar sind, versuche nach der nächsten Kategorie zu partitionieren
                partitionHierarchically(unknown_entries, categories, category_index + 1, tokenizer, result_partitions);
            } else {
                // Sonst erstelle eigene Partitionen für die unbekannten Einträge
                handleOnlyNoTokenEntries(unknown_entries, result_partitions);
            }
        }
    }
};

#endif // PARTITIONING_MNGR_H