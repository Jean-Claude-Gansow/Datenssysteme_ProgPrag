#ifndef PARTITIONING_MNGR_H
#define PARTITIONING_MNGR_H

#include <vector>
#include <cstdlib>
#include <cstring>
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
        printf("setting size-Threshhold for partitioning auf %zu\n", this->size_threshhold);
    }

    // Wrapper-Funktion, die das Eingabe-Dataset in ein Dataset von Partitionen umwandelt
    dataSet<partition_t>* create_partitions(dataSet<InType>* input_data, Tokenizer* tokenizer, const std::vector<category>& class_hierarchy) {
        printf("Creating partitions for dataset with %zu elements\n", input_data->size);
        
        // Erstelle eine initiale Partition mit allen Daten
        partition_t initial_partition;
        initial_partition.size = input_data->size;
        initial_partition.capacity = input_data->size;
        initial_partition.data = new pair[input_data->size];
        
        // Fülle die initiale Partition mit Paaren (Index, Pointer)
        for (size_t i = 0; i < input_data->size; i++) {
            initial_partition.data[i][0] = (uintptr_t)i;                   // Index
            initial_partition.data[i][1] = (uintptr_t)&input_data->data[i]; // Pointer
        }
        
        // Erstelle das Rückgabe-Dataset und initialisiere es mit der initialen Partition
        dataSet<partition_t>* result = new dataSet<partition_t>();
        result->size = 1;
        result->data = new partition_t[1];
        result->data[0] = initial_partition;
        
        // Wenn die Hierarchie leer ist oder die Daten unter dem Threshold liegen, gib das Ergebnis direkt zurück
        if (class_hierarchy.empty() || input_data->size <= size_threshhold) {
            return result;
        }
        
        // Rufe die rekursive Partitionierungsfunktion auf
        dataSet<partition_t>* partitioned_result = recursive_partitioning(result, tokenizer, class_hierarchy, 0);
        
        // Gib das aufgeteilte Ergebnis zurück
        return partitioned_result;
    }

private:
    // Rekursive Funktion zur Aufteilung von Partitionen
    dataSet<partition_t>* recursive_partitioning(dataSet<partition_t>* storage, Tokenizer* tokenizer, const std::vector<category>& class_hierarchy, size_t hierarchy_index) 
    {
        // Wenn wir am Ende der Hierarchie sind, gib das Storage zurück
        if (hierarchy_index >= class_hierarchy.size()) {
            return storage;
        }
        
        // Bestimme die aktuelle Klasse für die Aufteilung
        category current_class = class_hierarchy[hierarchy_index];
        printf("Partitioning level %zu using category %d\n", hierarchy_index, current_class);
        
        // Bestimme die Anzahl der Tokens für die aktuelle Klasse
        size_t num_tokens = tokenizer->m_class_tokens_found[current_class];
        if (num_tokens == 0) 
        {
            printf("Warning: No tokens found for category %d, skipping this level\n", current_class);
            return recursive_partitioning(storage, tokenizer, class_hierarchy, hierarchy_index + 1);
        }
        
        printf("Found %zu tokens for category %d\n", num_tokens, current_class);
        
        // Zähle zuerst die Gesamtzahl der Partitionen
        size_t total_partitions = 0;
        
        // Vektor für alle resultierenden Partitionen
        std::vector<partition_t> result_partitions;
        
        // Für jede Partition im Storage
        for (size_t p_idx = 0; p_idx < storage->size; p_idx++) 
        {
            partition_t& current_partition = storage->data[p_idx];
            
            // Wenn die Partition kleiner als der Threshold ist oder wir am Ende der Hierarchie sind, behalte sie unverändert
            if (current_partition.size <= size_threshhold || hierarchy_index >= class_hierarchy.size() - 1) 
            {
                result_partitions.push_back(current_partition);
                total_partitions++;
                continue;
            }
            
            // Erstelle temporäre Partitionen für die Aufteilung
            partition_t* temp_partitions = new partition_t[num_tokens];
            size_t* temp_counts = new size_t[num_tokens](); // Zähler für jede Partition
            
            // Zähle zuerst, wie viele Einträge in jede Partition kommen
            for (size_t i = 0; i < current_partition.size; i++) 
            {
                InType* entry = (InType*)current_partition.data[i][1];
                token tok_value = (*entry)[current_class];
                
                if (tok_value == 0) 
                {
                    // Wenn der Token 0 ist (keine Information), zähle ihn für alle Partitionen
                    for (size_t t = 0; t < num_tokens; t++) 
                    {
                        temp_counts[t]++;
                    }
                } 
                else if (tok_value <= num_tokens) 
                {
                    // Zähle für die spezifische Partition (Token-Werte beginnen bei 1)
                    temp_counts[tok_value - 1]++;
                } 
                else 
                {
                    printf("Warning: Token value %hu exceeds number of tokens %zu for category %d\n", 
                           tok_value, num_tokens, current_class);
                }
            }
            
            // Initialisiere die temporären Partitionen mit der richtigen Größe
            for (size_t t = 0; t < num_tokens; t++) 
            {
                temp_partitions[t].size = 0;
                temp_partitions[t].capacity = temp_counts[t];
                temp_partitions[t].data = new pair[temp_counts[t]];
                printf("Temp partition %zu has capacity %zu\n", t, temp_counts[t]);
            }
            
            // Verteile die Einträge auf die temporären Partitionen
            for (size_t i = 0; i < current_partition.size; i++) {
                InType* entry = (InType*)current_partition.data[i][1];
                token tok_value = (*entry)[current_class];
                
                if (tok_value == 0) {
                    // Wenn der Token 0 ist, füge den Eintrag zu allen Partitionen hinzu
                    for (size_t t = 0; t < num_tokens; t++) {
                        size_t idx = temp_partitions[t].size++;
                        temp_partitions[t].data[idx] = current_partition.data[i];
                    }
                } else if (tok_value <= num_tokens) {
                    // Füge den Eintrag zur spezifischen Partition hinzu
                    size_t t = tok_value - 1; // Token-Werte beginnen bei 1
                    size_t idx = temp_partitions[t].size++;
                    temp_partitions[t].data[idx] = current_partition.data[i];
                }
            }
            
            // Erstelle temporäres Storage für die rekursive Verarbeitung
            dataSet<partition_t>* sub_storage = new dataSet<partition_t>();
            sub_storage->size = num_tokens;
            sub_storage->data = temp_partitions;
            
            // Rekursiv weitere Aufteilung für jede temporäre Partition
            dataSet<partition_t>* sub_partitioned = recursive_partitioning(sub_storage, tokenizer, class_hierarchy, hierarchy_index + 1);
            
            // Füge die Sub-Partitionen zum Ergebnis hinzu
            for (size_t i = 0; i < sub_partitioned->size; i++) {
                result_partitions.push_back(sub_partitioned->data[i]);
                total_partitions++;
            }
            
            // Bereinige
            delete[] temp_counts;
            delete[] sub_partitioned->data; // Nur das Array löschen, nicht die einzelnen Partitionen
            delete sub_partitioned;
            
            // Gib den Speicher der aktuellen Partition frei, da wir sie aufgeteilt haben
            delete[] current_partition.data;
        }
        
        // Erstelle das endgültige Ergebnis mit der richtigen Größe
        dataSet<partition_t>* new_storage = new dataSet<partition_t>();
        new_storage->size = result_partitions.size();
        new_storage->data = new partition_t[result_partitions.size()];
        
        // Kopiere alle Partitionen ins Ergebnis
        for (size_t i = 0; i < result_partitions.size(); i++) {
            new_storage->data[i] = result_partitions[i];
        }
        
        // Bereinige das ursprüngliche Storage
        delete[] storage->data;
        delete storage;
        
        return new_storage;
    }

    //storage: list of all partitions, will contain all split up partitions
    //tokenizer: tokenizer, holding all information about how many tokens are in which class
    //class_hierarchy: hierarchy of classes, used to split up the partitions
    //split up index: storage list index of the partition that his call is supposed to split up
    //hierarchy_index: index of the current hierarchy level, used to determine which class to split by

   
private:
    size_t class_token_product(Tokenizer *tkn, const std::vector<category> &class_hierarchy, size_t curclass) 
    {
        size_t sum = 1; 
        for(int i = curclass; i < class_hierarchy.size(); ++i)
        {
            sum *= tkn->m_class_tokens_found[class_hierarchy[i]]; //sum * amount of found token of class
        }
        return sum;
    }

private : 
    size_t size_threshhold;
};

#endif // PARTITIONING_MNGR_H