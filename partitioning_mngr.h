#ifndef PARTITIONING_MNGR_H
#define PARTITIONING_MNGR_H

#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include "DataTypes.h"
#include "Tokenization_mngr.h"

/**
 * @brief Baumknoten für hierarchische Partitionierung mit separater Partition für Einträge ohne Information
 */
struct PartitionNode {
    partition_t data;              // Hauptpartition mit Einträgen, die Token-Information haben
    partition_t no_info;           // Separate Partition für Einträge ohne Token-Information
    PartitionNode** children;      // Kinder-Knoten
    size_t child_count;            // Anzahl der Kinder
    
    PartitionNode(const partition_t& p) : data(p), children(nullptr), child_count(0) {
        no_info.data = nullptr;
        no_info.size = 0;
        no_info.capacity = 0;
    }
    
    ~PartitionNode() {
        if (children) {
            for (size_t i = 0; i < child_count; i++) {
                delete children[i];
            }
            delete[] children;
        }
        
        if (no_info.data) {
            delete[] no_info.data;
        }
    }
};


template <typename base_type, typename InType, size_t N>
class Partitioning_mngr 
{
public:
    using Tokenizer = Tokenization_mngr<N, base_type, InType>;

    // Konfigurationsparameter
    struct Config {
        size_t size_threshold = 2000;     // Maximale Partitionsgröße
        bool verbose_logging = false;     // Detaillierte Ausgaben aktivieren
        double overlap_ratio = 0.5;       // 50% Überlappung bei finaler Aufteilung
        size_t min_token_count = 0;       // Minimale Anzahl von Token-Informationen
    };

    // Konstruktor
    explicit Partitioning_mngr(const Config& config = Config()) : config(config) {
        printf("Partitioning_mngr initialisiert mit Schwellwert %zu, Min-Token-Count %zu\n", 
               config.size_threshold, config.min_token_count);
    }

    /**
     * @brief Erstellt hierarchische Partitionen aus einem Datensatz
     */
    dataSet<partition_t>* create_partitions(
        dataSet<InType>* input_data, 
        Tokenizer* tokenizer, 
        const std::vector<category>& categories) 
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        printf("🔍 Partitionierung: Beginne mit %zu Einträgen und %zu Kategorien...\n", 
               input_data->size, categories.size());
        
        if (!input_data || input_data->size == 0) {
            return createEmptyPartitionSet();
        }
        
        if (input_data->size <= config.size_threshold) {
            return createSinglePartition(input_data);
        }

        printf("🔍 Erstelle gefilterte Basispartition (min. %zu Tokens)...\n", config.min_token_count);
        partition_t base_partition = createBasePartition(input_data);
        
        printf("🔍 Starte hierarchische Partitionierung mit %zu Kategorien...\n", categories.size());
        printf("   Kategoriereihenfolge: ");
        for (size_t i = 0; i < categories.size(); i++) {
            printf("%d", categories[i]);
            if (i < categories.size() - 1) printf(" → ");
        }
        printf("\n");
        
        // Erstelle den Wurzelknoten des Baums
        PartitionNode* root = new PartitionNode(base_partition);
        
        // Starte die hierarchische Partitionierung
        hierarchicalPartitioning(root, tokenizer, categories, 0);
        
        printf("✅ Hierarchische Partitionierung abgeschlossen\n");
        
        // Sammle alle Blätter des Baums (fertige Partitionen)
        size_t leaf_count = countLeaves(root);
        printf("🔍 %zu Blattpartitionen gefunden\n", leaf_count);
        
        // Sammle alle Blattpartitionen und Einträge ohne Token-Information
        partition_t* leaf_partitions = new partition_t[leaf_count];
        partition_t* no_info_partitions = new partition_t[leaf_count];
        size_t leaf_index = 0;
        size_t no_info_count = 0;
        
        collectLeaves(root, leaf_partitions, no_info_partitions, leaf_index, no_info_count);
        
        printf("🔍 %zu Blattpartitionen und %zu Partitionen mit Einträgen ohne Token-Information gesammelt\n", 
               leaf_index, no_info_count);
        
        // Verteile die Einträge ohne Token-Information auf die Blattpartitionen
        printf("🔍 Verteile Einträge ohne Token-Information auf Blattpartitionen...\n");
        distributeNoInfoEntries(no_info_partitions, no_info_count, leaf_partitions, leaf_index);
        
        // Nun überlappende Partitionen für alle noch zu großen Partitionen erstellen
        printf("🔍 Erstelle überlappende Partitionen für zu große Partitionen...\n");
        
        // Zähle zuerst, wie viele Partitionen nach dem Aufteilen entstehen werden
        size_t final_count = 0;
        for (size_t i = 0; i < leaf_index; i++) {
            if (leaf_partitions[i].size > config.size_threshold) {
                // Berechne, wie viele Teilpartitionen benötigt werden
                double overlap = config.overlap_ratio;
                size_t target_size = config.size_threshold;
                size_t step_size = static_cast<size_t>(target_size * (1.0 - overlap));
                if (step_size < 1) step_size = 1;
                size_t num_parts = (leaf_partitions[i].size + step_size - 1) / step_size;
                final_count += num_parts;
            } else {
                final_count++;
            }
        }
        
        // Erstelle das Ergebnis-Array
        partition_t* final_partitions = new partition_t[final_count];
        size_t final_index = 0;
        
        // Teile große Partitionen auf und füge sie zum Ergebnis hinzu
        for (size_t i = 0; i < leaf_index; i++) {
            if (leaf_partitions[i].size > config.size_threshold) {
                printf("   Partition %zu: %zu Einträge (über Schwellwert), erstelle überlappende Teilpartitionen\n", 
                       i, leaf_partitions[i].size);
                
                createOverlappingPartitions(leaf_partitions[i], final_partitions, final_index);
            } else {
                printf("   Partition %zu: %zu Einträge (unter Schwellwert), übernehme unverändert\n", 
                       i, leaf_partitions[i].size);
                
                final_partitions[final_index++] = leaf_partitions[i];
            }
        }
        
        // Erstelle das Ergebnisdatenset
        dataSet<partition_t>* result = new dataSet<partition_t>();
        result->size = final_index;
        result->data = new partition_t[final_index];
        
        for (size_t i = 0; i < final_index; i++) {
            result->data[i] = final_partitions[i];
        }
        
        // Aufräumen
        delete[] final_partitions;
        delete[] leaf_partitions;
        delete[] no_info_partitions;
        delete root; // Lösche den gesamten Baum
        
        // End-Zeit erfassen und Dauer berechnen
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        
        printf("✅ Partitionierung abgeschlossen: %zu Partitionen erstellt (%.2f Sekunden)\n", 
               result->size, duration / 1000.0);
        
        // Statistiken ausgeben
        size_t min_size = (result->size > 0) ? result->data[0].size : 0;
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
        
        return result;
    }

private:
    Config config;
    size_t partition_counter = 0;
    
    /**
     * @brief Erstellt die initiale Basispartition mit gefilterten Einträgen
     */
    partition_t createBasePartition(dataSet<InType>* input_data) {
        // Zähle zuerst, wie viele Einträge mindestens min_token_count Tokens haben
        size_t valid_count = 0;
        for (size_t i = 0; i < input_data->size; i++) {
            if (input_data->data[i].token_count >= config.min_token_count) {
                valid_count++;
            }
        }
        
        printf("🔍 Von %zu Einträgen haben %zu mindestens %zu Tokens (%.1f%%)\n", 
               input_data->size, valid_count, config.min_token_count,
               (valid_count * 100.0) / input_data->size);
        
        partition_t partition;
        partition.size = valid_count;
        partition.capacity = valid_count;
        partition.data = new pair[valid_count]();
        
        size_t valid_idx = 0;
        size_t report_step = input_data->size > 10000 ? input_data->size / 10 : input_data->size;
        
        for (size_t i = 0; i < input_data->size; i++) {
            if (i > 0 && i % report_step == 0) {
                printf("   Basispartition: %zu%% abgeschlossen (%zu/%zu)\n", (i * 100) / input_data->size, i, input_data->size);
            }
            
            if (input_data->data[i].token_count >= config.min_token_count) {
                partition.data[valid_idx][0] = static_cast<uintptr_t>(i);
                partition.data[valid_idx][1] = reinterpret_cast<uintptr_t>(&input_data->data[i]);
                valid_idx++;
            }
        }
        
        printf("✅ Basispartition erstellt mit %zu von %zu Einträgen (Kapazität: %zu)\n", 
               valid_count, input_data->size, partition.capacity);
        
        return partition;
    }
    
    /**
     * @brief Führt eine hierarchische Partitionierung basierend auf Kategorien durch
     */
    void hierarchicalPartitioning(
        PartitionNode* node,
        Tokenizer* tokenizer,
        const std::vector<category>& categories,
        size_t category_index)
    {
        partition_t& current_partition = node->data;
        
        // Aktuellen Status ausgeben
        if (category_index < categories.size()) {
            printf("🔄 Partitionierung (Tiefe %zu): Kategorie %d, %zu Elemente\n", 
                   category_index, categories[category_index], current_partition.size);
        }
        
        // Wenn keine weiteren Kategorien vorhanden, Partition im Baum lassen
        if (category_index >= categories.size()) {
            printf("   ✓ Keine weiteren Kategorien, Partition mit %zu Einträgen fertig\n", 
                   current_partition.size);
            partition_counter++;
            return;
        }
        
        // Aktuelle Kategorie auswählen
        category current_category = categories[category_index];
        
        // Anzahl der möglichen Token-Werte für diese Kategorie
        size_t num_tokens = tokenizer->m_class_tokens_found[current_category];
                
        printf("   🔍 Teile nach Kategorie %d (%zu verschiedene Token-Werte)\n", 
               current_category, num_tokens);
        
        // Array für Anzahl der Einträge pro Token
        size_t* token_counts = new size_t[num_tokens + 1]();  // Mit 0 initialisieren
        
        // Zähle zuerst die Anzahl der Einträge pro Token
        for (size_t i = 0; i < current_partition.size; i++) {
            InType* entry = reinterpret_cast<InType*>(current_partition.data[i][1]);
            token tok = (*entry)[current_category];
            
            if (tok >= 0 && tok <= num_tokens) {
                token_counts[tok]++;
            }
        }
        
        // Überprüfe, ob es Einträge ohne Token-Information gibt
        size_t no_info_count = token_counts[0];
        bool has_no_info = (no_info_count > 0);
        
        if (has_no_info) {
            printf("   ℹ️ %zu Einträge ohne Token-Information für Kategorie %d\n", 
                   no_info_count, current_category);
        }
        
        // Zähle nicht-leere Token-Werte
        size_t non_empty_count = 0;
        for (size_t t = 1; t <= num_tokens; t++) {
            if (token_counts[t] > 0) {
                non_empty_count++;
            }
        }
        
        printf("   ✓ %zu nicht-leere Subpartitionen gefunden\n", non_empty_count);
        
        if (non_empty_count == 0) {
            // Keine sinnvolle Aufteilung möglich
            printf("   ⚠️ Keine nicht-leeren Subpartitionen, überspringe Kategorie\n");
            delete[] token_counts;
            
            // Fahre mit der nächsten Kategorie fort
            hierarchicalPartitioning(node, tokenizer, categories, category_index + 1);
            return;
        }
        
        // Erstelle ein Array von PartitionNode-Pointern für die Kinder
        node->children = new PartitionNode*[non_empty_count];
        node->child_count = non_empty_count;
        
        // Array für nicht-leere Token-Werte
        size_t* non_empty_tokens = new size_t[non_empty_count];
        size_t token_idx = 0;
        
        // Erstelle Subpartitionen und speichere sie als Kinder
        for (size_t t = 1; t <= num_tokens; t++) {
            if (token_counts[t] > 0) {
                // Berechne Kapazität nur für Einträge mit Token-Information
                size_t capacity = token_counts[t];
                
                // Erstelle eine neue Partition
                partition_t sub_part;
                sub_part.capacity = capacity;
                sub_part.size = 0;
                sub_part.data = new pair[capacity]();
                
                printf("   ✓ Subpartition für Token %zu mit Kapazität %zu erstellt\n", 
                       t, capacity);
                
                // Erstelle einen neuen Knoten im Baum
                node->children[token_idx] = new PartitionNode(sub_part);
                non_empty_tokens[token_idx] = t;
                token_idx++;
            }
        }
        
        // Erstelle eine separate Partition für Einträge ohne Token-Information
        if (has_no_info) {
            node->no_info.capacity = no_info_count;
            node->no_info.size = 0;
            node->no_info.data = new pair[no_info_count]();
            printf("   ✓ Separate Partition für %zu Einträge ohne Token-Information erstellt\n", 
                   no_info_count);
        }
        
        // Status für Verteilung der Einträge
        size_t progress_step = current_partition.size > 1000 ? current_partition.size / 10 : current_partition.size;
        printf("   🔄 Verteile %zu Einträge auf %zu Subpartitionen und ggf. no_info-Partition...\n", 
               current_partition.size, non_empty_count);
        
        // Verteile die Einträge auf die Subpartitionen
        for (size_t i = 0; i < current_partition.size; i++) {
            if (i > 0 && i % progress_step == 0) {
                printf("      %zu%% abgeschlossen (%zu/%zu)\n", 
                       (i * 100) / current_partition.size, i, current_partition.size);
            }
            
            uintptr_t id = current_partition.data[i][0];
            InType* entry = reinterpret_cast<InType*>(current_partition.data[i][1]);
            token tok = (*entry)[current_category];
            
            if (tok >= 1 && tok <= num_tokens && token_counts[tok] > 0) {
                // Finde den richtigen Kindknoten
                for (size_t j = 0; j < non_empty_count; j++) {
                    if (non_empty_tokens[j] == tok) {
                        partition_t& target = node->children[j]->data;
                        target.data[target.size][0] = id;
                        target.data[target.size][1] = reinterpret_cast<uintptr_t>(entry);
                        target.size++;
                        break;
                    }
                }
            }
            else if (has_no_info) {
                // In die separate no_info Partition einfügen
                node->no_info.data[node->no_info.size][0] = id;
                node->no_info.data[node->no_info.size][1] = reinterpret_cast<uintptr_t>(entry);
                node->no_info.size++;
            }
        }
        
        printf("   ✓ Verteilung abgeschlossen\n");
        
        // Speicher der Elternpartition freigeben
        printf("   🗑️ Elternpartition mit %zu Einträgen wird gelöscht (Kapazität: %zu)\n", 
               current_partition.size, current_partition.capacity);
        delete[] current_partition.data;
        current_partition.data = nullptr;
        current_partition.size = 0;
        current_partition.capacity = 0;
        
        // Zeige Verteilungsstatistik
        printf("   📊 Verteilung nach Kategorie %d: ", current_category);
        for (size_t j = 0; j < non_empty_count && j < 5; j++) {
            size_t t = non_empty_tokens[j];
            printf("Token%zu=%zu ", t, node->children[j]->data.size);
        }
        if (non_empty_count > 5) {
            printf("... (%zu weitere)", non_empty_count - 5);
        }
        
        if (has_no_info) {
            printf(", No-Info=%zu", node->no_info.size);
        }
        printf("\n");
        
        // Rekursiv für alle Kinder aufrufen
        for (size_t j = 0; j < non_empty_count; j++) {
            size_t t = non_empty_tokens[j];
            printf("   🔄 Verarbeite Subpartition für Token %zu (Größe: %zu)\n", 
                   t, node->children[j]->data.size);
            
            hierarchicalPartitioning(node->children[j], tokenizer, categories, category_index + 1);
        }
        
        // Temporäre Arrays freigeben
        delete[] token_counts;
        delete[] non_empty_tokens;
    }
    
    // Zählt, wie viele Blätter der Baum hat
    size_t countLeaves(PartitionNode* node) {
        if (!node) return 0;
        
        // Ein Blatt ist ein Knoten ohne Kinder oder mit gelöschter Partition
        if (node->child_count == 0 && node->data.size > 0) {
            return 1;
        }
        
        size_t count = 0;
        for (size_t i = 0; i < node->child_count; i++) {
            count += countLeaves(node->children[i]);
        }
        
        return count;
    }

    // Sammelt alle Blätter und no_info Partitionen in Arrays
    void collectLeaves(
        PartitionNode* node, 
        partition_t* leaf_partitions, 
        partition_t* no_info_partitions,
        size_t& leaf_index, 
        size_t& no_info_index) 
    {
        if (!node) return;
        
        // Sammle no_info Partition, falls vorhanden
        if (node->no_info.data != nullptr && node->no_info.size > 0) {
            no_info_partitions[no_info_index++] = node->no_info;
            // Setze auf null, damit sie nicht doppelt freigegeben wird
            node->no_info.data = nullptr;
            node->no_info.size = 0;
            node->no_info.capacity = 0;
        }
        
        // Ein Blatt ist ein Knoten ohne Kinder oder mit gelöschter Partition
        if (node->child_count == 0 && node->data.size > 0) {
            leaf_partitions[leaf_index++] = node->data;
            // Setze auf null, damit sie nicht doppelt freigegeben wird
            node->data.data = nullptr;
            node->data.size = 0;
            node->data.capacity = 0;
            return;
        }
        
        for (size_t i = 0; i < node->child_count; i++) {
            collectLeaves(node->children[i], leaf_partitions, no_info_partitions, leaf_index, no_info_index);
        }
    }
    
    // Verteilt die Einträge ohne Token-Information auf die Blattpartitionen
    void distributeNoInfoEntries(
        partition_t* no_info_partitions, 
        size_t no_info_count,
        partition_t* leaf_partitions, 
        size_t leaf_count)
    {
        if (no_info_count == 0 || leaf_count == 0) {
            return;
        }
        
        // Zähle die Gesamtzahl der Einträge ohne Token-Information
        size_t total_no_info_entries = 0;
        for (size_t i = 0; i < no_info_count; i++) {
            total_no_info_entries += no_info_partitions[i].size;
        }
        
        printf("   🔍 Einfache Verteilung von %zu Einträgen ohne Token-Information...\n", 
               total_no_info_entries);
        
        // Berechne, wie viele Einträge pro Partition hinzugefügt werden müssen
        // (gleichmäßig auf alle Partitionen verteilt)
        size_t entries_per_partition = (total_no_info_entries + leaf_count - 1) / leaf_count;
        
        // Erweitere die Kapazität jeder Partition
        for (size_t i = 0; i < leaf_count; i++) {
            size_t old_size = leaf_partitions[i].size;
            size_t new_capacity = old_size + entries_per_partition;
            
            pair* new_data = new pair[new_capacity]();
            
            // Kopiere die bestehenden Einträge
            for (size_t j = 0; j < old_size; j++) {
                new_data[j][0] = leaf_partitions[i].data[j][0];
                new_data[j][1] = leaf_partitions[i].data[j][1];
            }
            
            // Ersetze die alten Daten
            delete[] leaf_partitions[i].data;
            leaf_partitions[i].data = new_data;
            leaf_partitions[i].capacity = new_capacity;
        }
        
        // Verteile die Einträge sequentiell
        size_t target_partition = 0;
        size_t entry_count = 0;
        
        for (size_t i = 0; i < no_info_count; i++) {
            for (size_t j = 0; j < no_info_partitions[i].size; j++) {
                // Wechsle zur nächsten Partition, wenn die aktuelle voll ist
                while (leaf_partitions[target_partition].size >= leaf_partitions[target_partition].capacity) {
                    target_partition = (target_partition + 1) % leaf_count;
                }
                
                // Füge den Eintrag hinzu
                leaf_partitions[target_partition].data[leaf_partitions[target_partition].size][0] = 
                    no_info_partitions[i].data[j][0];
                leaf_partitions[target_partition].data[leaf_partitions[target_partition].size][1] = 
                    no_info_partitions[i].data[j][1];
                leaf_partitions[target_partition].size++;
                
                // Wechsle zur nächsten Partition für gleichmäßige Verteilung
                target_partition = (target_partition + 1) % leaf_count;
                entry_count++;
                
                // Status-Update für große Datenmengen
                if (entry_count % 100000 == 0) {
                    printf("      %zu%% der Einträge ohne Token-Information verteilt (%zu/%zu)\n",
                           (entry_count * 100) / total_no_info_entries, entry_count, total_no_info_entries);
                }
            }
            
            // Gib den Speicher der verarbeiteten no_info Partition frei
            delete[] no_info_partitions[i].data;
            no_info_partitions[i].data = nullptr;
        }
        
        printf("   ✅ %zu Einträge ohne Token-Information wurden gleichmäßig auf %zu Partitionen verteilt\n",
               entry_count, leaf_count);
    }

    /**
     * @brief Erstellt überlappende Teilpartitionen für eine große Partition
     */
    void createOverlappingPartitions(
        partition_t& large_partition, 
        partition_t* result_partitions,
        size_t& result_index)
    {
        size_t size = large_partition.size;
        
        double overlap = config.overlap_ratio;
        size_t target_size = config.size_threshold;
        
        if (size <= target_size) {
            printf("   ✓ Partition ist bereits klein genug (%zu Einträge)\n", size);
            result_partitions[result_index++] = large_partition;
            return;
        }
        
        size_t step_size = static_cast<size_t>(target_size * (1.0 - overlap));
        if (step_size < 1) step_size = 1;
        
        size_t num_parts = (size + step_size - 1) / step_size;
        
        printf("   🔍 Erstelle %zu überlappende Teilpartitionen (Überlappung: %.0f%%) aus Partition mit %zu Einträgen\n", 
            num_parts, overlap * 100, size);
        
        for (size_t part = 0; part < num_parts; part++) {
            size_t start_idx = part * step_size;
            size_t end_idx = start_idx + target_size;
            
            if (end_idx > size) end_idx = size;
            
            size_t part_size = end_idx - start_idx;
            
            partition_t sub_part;
            sub_part.size = part_size;
            sub_part.capacity = part_size;
            sub_part.data = new pair[part_size]();
            
            for (size_t i = 0; i < part_size; i++) {
                size_t src_idx = start_idx + i;
                sub_part.data[i][0] = large_partition.data[src_idx][0];
                sub_part.data[i][1] = large_partition.data[src_idx][1];
            }
            
            result_partitions[result_index++] = sub_part;
            partition_counter++;
            
            if (part % 5 == 0 || part == num_parts - 1 || end_idx >= size) {
                printf("      ✓ Teilpartition %zu/%zu: %zu Einträge (Bereich %zu-%zu)\n", 
                    part + 1, num_parts, part_size, start_idx, end_idx - 1);
            }
            
            if (end_idx >= size) break;
        }
        
        printf("   🗑️ Große Partition mit %zu Einträgen wird nach Aufteilung gelöscht\n", size);
        delete[] large_partition.data;
    }
    
    /**
     * @brief Erstellt eine einzelne Partition mit allen Einträgen
     */
    dataSet<partition_t>* createSinglePartition(dataSet<InType>* input_data) {
        printf("🔍 Erstelle einzelne Partition für %zu Einträge...\n", input_data->size);
        
        partition_t single_partition = createBasePartition(input_data);
        
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