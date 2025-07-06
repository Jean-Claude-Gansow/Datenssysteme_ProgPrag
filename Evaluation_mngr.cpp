//
// Created by Jean-Claude on 19.05.2025.
//

#include "Evaluation_mngr.h"

float Evaluation_mngr::evaluateMatches(dataSet<match>* matches, dataSet<match>* Solution)
{
    size_t buffer_size = matches->size;
    size_t* num_matches = new size_t[matches->size];
    size_t **indexed_matches = new size_t*[matches->size];

    // Initialize num_matches array
    memset(num_matches, 0, matches->size * sizeof(size_t));

    //create a buffer, that holds all matches at the designated first id´s indexes
    for(unsigned int i = 0; i < buffer_size; i++)
    {
        indexed_matches[i] = new size_t[threshhold];
        memset(indexed_matches[i], 0, threshhold * sizeof(size_t));
        size_t id1 = (*matches)[i].data[0];
        size_t id2 = (*matches)[i].data[1];
        
        if(id1 > 0 && id1 <= matches->size && num_matches[id1-1] < threshhold) {
            indexed_matches[id1-1][num_matches[id1-1]] = id2; //sort matches to be associated with the interfering index
            num_matches[id1-1]++;
        }
    }

    // Check if each solution match exists in our predicted matches
    // Count true positives (TP), false positives (FP), and false negatives (FN)
    size_t true_positives = 0;
    size_t false_negatives = 0;
    
    for(unsigned int i = 0; i < Solution->size; i++)
    {
        size_t solution_id1 = (*Solution)[i].data[0];
        size_t solution_id2 = (*Solution)[i].data[1];
        
        if(solution_id1 > 0 && solution_id1 <= matches->size) {
            bool found = false;
            for(unsigned int j = 0; j < num_matches[solution_id1-1]; j++) {
                if(indexed_matches[solution_id1-1][j] == solution_id2) {
                    indexed_matches[solution_id1-1][j] = 0; // Mark as found (true positive)
                    true_positives++;
                    found = true;
                    break;
                }
            }
            
            if(!found) {
                false_negatives++; // Solution match not found in our predicted matches
            }
        } else {
            false_negatives++; // Solution id not in range of our predicted matches
        }
    }
    
    // Count remaining non-zero entries as false positives
    size_t false_positives = 0;
    for(unsigned int i = 0; i < matches->size; i++) {
        for(unsigned int j = 0; j < num_matches[i]; j++) {
            if(indexed_matches[i][j] != 0) {
                false_positives++;
            }
        }
    }
    
    // Calculate precision, recall, and F1 score
    float precision = 0.0f;
    if(true_positives + false_positives > 0) {
        precision = (float)true_positives / (true_positives + false_positives);
    }
    
    float recall = 0.0f;
    if(true_positives + false_negatives > 0) {
        recall = (float)true_positives / (true_positives + false_negatives);
    }
    
    float f1_score = 0.0f;
    if(precision + recall > 0) {
        f1_score = 2.0f * (precision * recall) / (precision + recall);
    }
    
    // Clean up allocated memory
    for(unsigned int i = 0; i < matches->size; i++) {
        delete[] indexed_matches[i];
    }
    delete[] indexed_matches;
    delete[] num_matches;
    
    return f1_score;
}