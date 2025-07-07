//
// Created by Jean-Claude on 19.05.2025.
//

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
    
    // Erstelle einen Puffer für die indizierten Matches
    size_t buffer_size = max_id + 1; // +1 weil IDs bei 1 starten können
    size_t* num_matches = new size_t[buffer_size];
    size_t** indexed_matches = new size_t*[buffer_size];

    // Initialisiere das num_matches-Array
    memset(num_matches, 0, buffer_size * sizeof(size_t));

    // Erstelle den Puffer für alle Matches an den zugehörigen ersten ID-Indizes
    for (size_t i = 0; i < buffer_size; i++) {
        indexed_matches[i] = new size_t[threshhold];
        memset(indexed_matches[i], 0, threshhold * sizeof(size_t));
    }
    
    // Befülle den Puffer mit allen Matches aus allen Partitionen
    for (size_t p = 0; p < matches->size; p++) {
        matching& partition_matches = matches->data[p];
        
        for (size_t m = 0; m < partition_matches.size; m++) {
            size_t id1 = static_cast<size_t>(partition_matches.matches[m].data[0]);
            size_t id2 = static_cast<size_t>(partition_matches.matches[m].data[1]);
            
            if (id1 > 0 && id1 < buffer_size && num_matches[id1-1] < threshhold) {
                indexed_matches[id1-1][num_matches[id1-1]] = id2; // Sortiere Matches nach dem entsprechenden Index
                num_matches[id1-1]++;
            }
        }
    }

    // Überprüfe, ob jedes Lösungs-Match in unseren vorhergesagten Matches existiert
    // Zähle true positives (TP), false positives (FP) und false negatives (FN)
    size_t true_positives = 0;
    size_t false_negatives = 0;
    
    for (size_t i = 0; i < Solution->size; i++) {
        size_t solution_id1 = static_cast<size_t>((*Solution)[i].data[0]);
        size_t solution_id2 = static_cast<size_t>((*Solution)[i].data[1]);
        
        if (solution_id1 < buffer_size) 
        {
            bool found = false;
            for (size_t j = 0; j < num_matches[solution_id1-1]; j++) {
                if (indexed_matches[solution_id1-1][j] == solution_id2) {
                    indexed_matches[solution_id1-1][j] = 0; // Als gefunden markieren (true positive)
                    true_positives++;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                false_negatives++; // Lösungs-Match nicht in unseren vorhergesagten Matches gefunden
            }
        } else {
            false_negatives++; // Lösungs-ID nicht im Bereich unserer vorhergesagten Matches
        }
    }
    
    // Zähle verbleibende Nicht-Null-Einträge als false positives
    size_t false_positives = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        for (size_t j = 0; j < num_matches[i]; j++) {
            if (indexed_matches[i][j] != 0) {
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
    
    // Gib den zugewiesenen Speicher frei
    for (size_t i = 0; i < buffer_size; i++) {
        delete[] indexed_matches[i];
    }
    delete[] indexed_matches;
    delete[] num_matches;
    
    return f1_score;
}