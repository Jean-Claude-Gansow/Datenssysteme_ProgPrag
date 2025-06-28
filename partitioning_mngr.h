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

    // Gibt ein dataSet<partition> zurück, jede Partition enthält Paare (ID, Pointer)
    dataSet<partition>* partition_iterative(const dataSet<InType>& input, const std::vector<category>& class_hierarchy, Tokenizer* tokenizer) 
    {
        // Initial: eine Partition mit allen Einträgen
        std::vector<std::vector<const InType*>> current_partitions;
        std::vector<const InType*> initial;
        initial.reserve(input.size);
        for (size_t i = 0; i < input.size; ++i)
            initial.push_back(&input.data[i]);
        current_partitions.push_back(std::move(initial));

        // Iterativ für jede Kategorie partitionieren
        for (size_t level = 0; level < class_hierarchy.size(); ++level) {
            category cat = class_hierarchy[level];
            size_t num_tokens = tokenizer->m_class_tokens_found[cat];

            std::vector<std::vector<const InType*>> next_partitions;

            for (const auto& part : current_partitions) {
                // Zähle pro Tokenwert
                std::vector<size_t> counts(num_tokens, 0);
                for (const InType* entry : part) {
                    token t = (*entry)[cat];
                    if (t >= num_tokens) t = 0;
                    counts[t]++;
                }
                // Speicher für Pointer anlegen
                std::vector<std::vector<const InType*>> buffers(num_tokens);
                for (size_t t = 0; t < num_tokens; ++t)
                    buffers[t].reserve(counts[t]);
                // Einsortieren
                for (const InType* entry : part) {
                    token t = (*entry)[cat];
                    if (t >= num_tokens) t = 0;
                    buffers[t].push_back(entry);
                }
                // Neue Partitionen anlegen
                for (size_t t = 0; t < num_tokens; ++t) {
                    if (!buffers[t].empty())
                        next_partitions.push_back(std::move(buffers[t]));
                }
            }
            current_partitions = std::move(next_partitions);
        }

        // Jetzt current_partitions in echte Partitionen (mit pair) umwandeln
        dataSet<partition>* result = new dataSet<partition>();
        result->size = current_partitions.size();
        result->data = (partition*)malloc(result->size * sizeof(partition));
        for (size_t i = 0; i < result->size; ++i) {
            const auto& part = current_partitions[i];
            result->data[i].size = part.size();
            result->data[i].data = (pair*)malloc(part.size() * sizeof(pair));
            for (size_t j = 0; j < part.size(); ++j) {
                result->data[i].data[j].data[0] = part[j]->id;         // ID
                result->data[i].data[j].data[1] = (uintptr_t)part[j];  // Pointer
            }
        }
        return result;
    }
};

#endif // PARTITIONING_MNGR_H