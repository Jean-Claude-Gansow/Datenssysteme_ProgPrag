#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include "Matching_mngr.h"
#include "DataTypes.h"
#include "debug_utils.h"

// Teststruktur für compType
class TestCompType {
private:
    uint64_t id;
    std::vector<std::string> tokens;
    
public:
    TestCompType(uint64_t id, const std::vector<std::string>& tokens) 
        : id(id), tokens(tokens) {}
    
    // Operator == implementieren für Matching
    // 0: Kein Match
    // 1: Match
    // 2: Fallback (Ähnlichkeitsberechnung erforderlich)
    int operator==(const TestCompType& other) const {
        // Für Test-Zwecke: Wenn ID % 3 == 0, dann ist es ein direkter Match
        if (id % 3 == 0 && other.id % 3 == 0) {
            return 1; // Direkter Match
        } 
        // Wenn ID % 5 == 0, dann nutzen wir den Fallback-Pfad
        else if (id % 5 == 0 || other.id % 5 == 0) {
            return 2; // Fallback
        }
        else {
            return 0; // Kein Match
        }
    }
    
    // Operator | implementieren für Ähnlichkeitsberechnung
    double operator|(const TestCompType& other) const {
        // Berechnung eines Ähnlichkeitswerts zwischen 0.0 und 1.0
        // Für Test-Zwecke: Ein höherer Wert, wenn die IDs näher beieinander liegen
        double similarity = 1.0 - (std::abs(static_cast<double>(id) - static_cast<double>(other.id)) / 100.0);
        
        // Beschränken auf [0,1]
        if (similarity < 0.0) similarity = 0.0;
        if (similarity > 1.0) similarity = 1.0;
        
        return similarity;
    }
};

// Hilfsfunktion zum Erstellen einer Testpartition
partition_t create_test_partition(size_t size) {
    partition_t part;
    part.size = size;
    part.data = new pair[size];
    
    for (size_t i = 0; i < size; i++) {
        // ID ist i+1 (1-basiert)
        part.data[i][0] = static_cast<uintptr_t>(i+1);
        
        // Erstelle Testobjekt
        std::vector<std::string> tokens = {"token1", "token2", "token" + std::to_string(i)};
        TestCompType* comp = new TestCompType(i+1, tokens);
        part.data[i][1] = reinterpret_cast<uintptr_t>(comp);
    }
    
    return part;
}

// Hilfsfunktion zum Freigeben des Speichers einer Partition
void free_test_partition(partition_t& part) {
    for (size_t i = 0; i < part.size; i++) {
        TestCompType* comp = reinterpret_cast<TestCompType*>(part.data[i][1]);
        delete comp;
    }
    delete[] part.data;
    part.data = nullptr;
    part.size = 0;
}

// Test für die match_blocker_intern-Funktion
void test_match_blocker() {
    std::cout << "=== Testing match_blocker_intern ===" << std::endl;
    
    // Erstelle Testpartition
    partition_t part = create_test_partition(10);
    
    // Erstelle Matching-Manager
    Matching_mngr<TestCompType> manager;
    
    // Ergebnispuffer
    matching_t results;
    results.size = 0;
    results.matches = nullptr;
    
    // Führe Matching durch
    manager.match_blocker_intern(&part, 0, part.size, &results);
    
    // Überprüfe, ob es Matches gibt
    std::cout << "Found " << results.size << " matches" << std::endl;
    assert(results.size > 0);
    
    // Überprüfe die gefundenen Matches
    for (size_t i = 0; i < results.size; i++) {
        uintptr_t id1 = results.matches[i].data[0];
        uintptr_t id2 = results.matches[i].data[1];
        
        std::cout << "Match " << i+1 << ": " << id1 << " with " << id2 << std::endl;
        
        // Überprüfe, dass die IDs nicht 0 sind
        assert(id1 != 0);
        assert(id2 != 0);
    }
    
    // Speicher freigeben
    delete[] results.matches;
    free_test_partition(part);
    
    std::cout << "Test passed!" << std::endl;
}

// Test für die identify_matches-Funktion mit Multi-Threading
void test_identify_matches_multithreaded() {
    std::cout << "\n=== Testing identify_matches with multiple threads ===" << std::endl;
    
    // Erstelle dataSet mit Partitionen
    dataSet<partition_t>* input = new dataSet<partition_t>();
    input->size = 2;
    input->data = new partition_t[2];
    
    // Erstelle zwei Testpartitionen
    input->data[0] = create_test_partition(20);
    input->data[1] = create_test_partition(15);
    
    // Erstelle Matching-Manager
    Matching_mngr<TestCompType> manager;
    
    // Führe Multi-Thread-Matching durch
    dataSet<match>* result = manager.identify_matches(input, 4);
    
    // Überprüfe, ob es Ergebnisse gibt
    assert(result != nullptr);
    // Hier müssen wir die Erwartung anpassen, da wir jetzt direkt matches zurückgeben
    // und nicht mehr Partitionen mit matches
    
    // Wir haben jetzt direkt Zugriff auf die Matches, statt auf Partitionen
    std::cout << "Total matches found: " << result->size << std::endl;
    
    // Überprüfe jedes Match
    for (size_t i = 0; i < result->size; i++) {
        uintptr_t id1 = result->data[i].data[0];
        uintptr_t id2 = result->data[i].data[1];
        
        // Überprüfe, dass die IDs nicht 0 sind
        assert(id1 != 0);
        assert(id2 != 0);
        
        std::cout << "Match " << i << ": " << id1 << " <-> " << id2 << std::endl;
    }
    // Speicher freigeben - jetzt einfacher, da wir direkt mit dataSet<match> arbeiten
    
    // Speicher freigeben
    for (size_t i = 0; i < input->size; i++) {
        free_test_partition(input->data[i]);
    }
    delete[] input->data;
    delete input;
    
    delete[] result->data;
    delete result;
    
    std::cout << "Test passed!" << std::endl;
}

// Test für die Fallback-Logik
void test_fallback_matching() {
    std::cout << "\n=== Testing fallback matching logic ===" << std::endl;
    
    // Erstelle spezielle Testpartition mit Elementen, die den Fallback-Pfad auslösen
    partition_t part;
    part.size = 10;
    part.data = new pair[part.size];
    
    for (size_t i = 0; i < part.size; i++) {
        // ID ist i+1 (1-basiert)
        part.data[i][0] = static_cast<uintptr_t>(i+1);
        
        // Erstelle Testobjekt - alle IDs sind durch 5 teilbar für Fallback-Tests
        std::vector<std::string> tokens = {"token1", "token2", "token" + std::to_string(i)};
        TestCompType* comp = new TestCompType(i*5, tokens);  // ID ist i*5 (z.B. 0, 5, 10, 15...)
        part.data[i][1] = reinterpret_cast<uintptr_t>(comp);
    }
    
    // Erstelle Matching-Manager
    Matching_mngr<TestCompType> manager;
    
    // Ergebnispuffer
    matching_t results;
    results.size = 0;
    results.matches = nullptr;
    
    // Führe Matching durch
    manager.match_blocker_intern(&part, 0, part.size, &results);
    
    // Überprüfe, ob es Matches gibt (sollten über Fallback-Pfad gefunden werden)
    std::cout << "Found " << results.size << " fallback matches" << std::endl;
    
    // Speicher freigeben
    if (results.matches) {
        delete[] results.matches;
    }
    
    free_test_partition(part);
    
    std::cout << "Fallback test completed!" << std::endl;
}

int main() {
    std::cout << "Starting Matching Manager tests...\n" << std::endl;
    
    test_match_blocker();
    test_identify_matches_multithreaded();
    test_fallback_matching();
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}
