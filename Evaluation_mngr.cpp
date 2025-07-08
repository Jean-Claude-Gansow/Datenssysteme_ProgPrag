//
// Created by Jean-Claude on 19.05.2025.
//
#include <vector>
#include "Evaluation_mngr.h"

float Evaluation_mngr::evaluateMatches(dataSet<matching>* matches, dataSet<match>* Solution)
{
    // Bestimme die maximale ID, um einen geeigneten Puffer zu erstellen
    size_t max_id = 0;
    size_t total_matches = 0;
    
    // Zähle zuerst die Gesamtzahl der Matches und finde die maximale ID
    for (size_t p = 0; p < matches->size; p++) {
        matching& partition_matches = matches->data[p];
        total_matches += partition_matches.size;
        
        for (size_t i = 0; i < partition_matches.size; i++) {
            size_t id1 = static_cast<size_t>(partition_matches.matches[i].data[0]);
            size_t id2 = static_cast<size_t>(partition_matches.matches[i].data[1]);
            max_id = std::max(max_id, std::max(id1, id2));
        }
    }
    
    // Debugging-Informationen
    printf("Ground-Truth-Größe: %zu\n", Solution->size);
    printf("Anzahl verarbeiteter Matches: %zu\n", total_matches);
    printf("Maximale gefundene ID: %zu\n", max_id);
    
    // Erstelle einen Puffer für die indizierten Matches
    size_t buffer_size = max_id + 1;
    
    // Verwende ein Set von Matches pro ID, um Duplikate zu vermeiden
    std::vector<std::unordered_set<size_t>> indexed_matches(buffer_size);
    
    // Befülle den Puffer mit allen Matches aus allen Partitionen
    for (size_t p = 0; p < matches->size; p++) {
        matching& partition_matches = matches->data[p];
        
        for (size_t m = 0; m < partition_matches.size; m++) {
            size_t id1 = static_cast<size_t>(partition_matches.matches[m].data[0]);
            size_t id2 = static_cast<size_t>(partition_matches.matches[m].data[1]);
            
            // Keine Begrenzung mehr durch threshhold
            if (id1 < buffer_size) {
                indexed_matches[id1].insert(id2); // Verwende insert statt push_back, um Duplikate zu vermeiden
            }
        }
    }

    // Überprüfe, ob jedes Lösungs-Match in unseren vorhergesagten Matches existiert
    size_t true_positives = 0;
    size_t false_negatives = 0;
    
    // Erstelle ein Set für die bereits als true positive markierten Matches
    std::vector<std::unordered_set<size_t>> marked_matches(buffer_size);
    
    for (size_t i = 0; i < Solution->size; i++) {
        size_t solution_id1 = static_cast<size_t>((*Solution)[i].data[0]);
        size_t solution_id2 = static_cast<size_t>((*Solution)[i].data[1]);
        
        if (solution_id1 < buffer_size) {
            if (indexed_matches[solution_id1].find(solution_id2) != indexed_matches[solution_id1].end()) {
                // Als gefunden markieren
                marked_matches[solution_id1].insert(solution_id2);
                true_positives++;
            } else {
                false_negatives++;
            }
        } else {
            false_negatives++;
        }
    }
    
    // Zähle alle Matches, die nicht als true positive markiert wurden, als false positives
    size_t false_positives = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        for (const auto& match_id : indexed_matches[i]) {
            if (marked_matches[i].find(match_id) == marked_matches[i].end()) {
                false_positives++;
            }
        }
    }
    
    // Berechne Precision, Recall und F1-Score
    float precision = 0.0f;
    if (true_positives + false_positives > 0) {
        precision = static_cast<float>(true_positives) / (true_positives + false_positives);
    }
    
    float recall = 0.0f;
    if (true_positives + false_negatives > 0) {
        recall = static_cast<float>(true_positives) / (true_positives + false_negatives);
    }
    
    float f1_score = 0.0f;
    if (precision + recall > 0) {
        f1_score = 2.0f * (precision * recall) / (precision + recall);
    }
    
    // Debugging-Ausgabe
    printf("\n✓ Evaluationsergebnisse:\n");
    printf("  - Wahre Positive (TP): %zu\n", true_positives);
    printf("  - Falsche Positive (FP): %zu\n", false_positives);
    printf("  - Falsche Negative (FN): %zu\n", false_negatives);
    printf("  - Precision: %.4f\n", precision);
    printf("  - Recall: %.4f\n", recall);
    printf("  - F1-Score: %.4f\n", f1_score);
    
    return f1_score;
}