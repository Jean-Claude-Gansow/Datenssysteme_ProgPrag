#include <iostream>
#include <vector>
#include <cstring>
#include <cassert>
#include "../Matching_mngr.h"
#include "../DataTypes.h"

// Einfache Teststruktur für compType
struct TestData {
    int value;
    
    // Operator == für Matching
    int operator==(const TestData& other) const {
        if (value == other.value) return 1; // Match
        if (std::abs(value - other.value) < 5) return 2; // Fallback
        return 0; // Kein Match
    }
    
    // Operator | für Ähnlichkeitsberechnung
    double operator|(const TestData& other) const {
        int diff = std::abs(value - other.value);
        return 1.0 - diff / 10.0;
    }
};

// Test-Partition erstellen
partition_t createTestPartition(size_t size) {
    partition_t part;
    part.size = size;
    part.data = new pair[size];
    
    // Mit Testdaten füllen
    for (size_t i = 0; i < size; i++) {
        part.data[i][0] = i + 1;  // ID ist 1-basiert
        TestData* data = new TestData();
        data->value = static_cast<int>(i);
        part.data[i][1] = reinterpret_cast<uintptr_t>(data);
    }
    
    return part;
}

// Test-Partitionen freigeben
void freeTestPartition(partition_t& part) {
    if (part.data) {
        for (size_t i = 0; i < part.size; i++) {
            if (part.data[i][1] != 0) {
                TestData* data = reinterpret_cast<TestData*>(part.data[i][1]);
                delete data;
            }
        }
        delete[] part.data;
        part.data = nullptr;
    }
    part.size = 0;
}

// DataSet erstellen
dataSet<partition_t>* createTestDataSet(size_t numPartitions, size_t partitionSize) {
    dataSet<partition_t>* set = new dataSet<partition_t>();
    set->size = numPartitions;
    set->data = new partition_t[numPartitions];
    
    for (size_t i = 0; i < numPartitions; i++) {
        set->data[i] = createTestPartition(partitionSize);
    }
    
    return set;
}

// DataSet freigeben
void freeTestDataSet(dataSet<partition_t>* set) {
    if (!set) return;
    
    if (set->data) {
        for (size_t i = 0; i < set->size; i++) {
            freeTestPartition(set->data[i]);
        }
        delete[] set->data;
    }
    
    delete set;
}

// Überprüfen des Speicherzugriffs
bool testMemoryAccess(Matching_mngr<TestData>& manager, dataSet<partition_t>* set) {
    // Ein einzelnes Match durchführen
    if (set && set->size > 0 && set->data[0].size > 0) {
        matching_t result;
        result.size = 0;
        result.matches = nullptr;
        
        manager.match_blocker_intern(&set->data[0], 0, set->data[0].size, &result);
        
        std::cout << "Found " << result.size << " matches in first partition" << std::endl;
        
        // Ergebnis überprüfen
        if (result.matches) {
            // Ergebnis-Matches ausgeben
            for (size_t i = 0; i < result.size && i < 10; i++) {
                std::cout << "  Match: " << result.matches[i].data[0] << " with " << result.matches[i].data[1] << std::endl;
            }
            
            // Speicher freigeben
            delete[] result.matches;
            return true;
        } else {
            std::cout << "No matches found or memory allocation failed" << std::endl;
        }
    }
    return false;
}

// Test mit Multi-Threading
bool testMultiThreading(Matching_mngr<TestData>& manager, dataSet<partition_t>* set) {
    if (!set || set->size == 0) return false;
    
    std::cout << "Testing multi-threading with " << set->size << " partitions..." << std::endl;
    
    // Alle Partitionen verarbeiten
    dataSet<match>* results = manager.identify_matches(set, 4);
    
    if (!results) {
        std::cout << "Error: identify_matches returned nullptr" << std::endl;
        return false;
    }
    
    std::cout << "Successfully processed " << results->size << " matches" << std::endl;
    
    // Wir haben jetzt direkt matches statt Partitionen
    std::cout << "Total matches: " << results->size << std::endl;
    
    // Ersten 10 Matches ausgeben
    for (size_t i = 0; i < results->size && i < 10; i++) {
        std::cout << "  Match " << i << ": " << results->data[i].data[0] << " with " 
                 << results->data[i].data[1] << std::endl;
    }
        }
    }
    
    // Speicher für results freigeben
    if (results->data) {
        delete[] results->data;
    }
    delete results;
    
    return true;
}

// Extremfall-Test: Sehr große Partition
bool testLargePartition(Matching_mngr<TestData>& manager) {
    std::cout << "Testing with a large partition..." << std::endl;
    
    // Erstelle eine Partition mit 1000 Elementen
    partition_t part = createTestPartition(1000);
    
    // Match durchführen
    matching_t result;
    result.size = 0;
    result.matches = nullptr;
    
    manager.match_blocker_intern(&part, 0, part.size, &result);
    
    std::cout << "Found " << result.size << " matches in large partition" << std::endl;
    
    // Speicher freigeben
    if (result.matches) {
        delete[] result.matches;
    }
    
    freeTestPartition(part);
    return true;
}

// Negativfall-Test: Ungültige Partition
bool testInvalidPartition(Matching_mngr<TestData>& manager) {
    std::cout << "Testing with invalid partition..." << std::endl;
    
    // Leere Partition
    partition_t part;
    part.size = 0;
    part.data = nullptr;
    
    matching_t result;
    result.size = 0;
    result.matches = nullptr;
    
    manager.match_blocker_intern(&part, 0, 0, &result);
    
    // Sollte keine Abstürze verursachen
    std::cout << "Invalid partition test completed without crashing" << std::endl;
    return true;
}

int main() {
    std::cout << "Starting memory check for Matching_mngr..." << std::endl;
    
    // Manager erstellen
    Matching_mngr<TestData> manager;
    
    // Test-Daten erstellen: 5 Partitionen mit je 100 Elementen
    dataSet<partition_t>* testData = createTestDataSet(5, 100);
    
    bool allTestsPassed = true;
    
    // Test 1: Einzelner Match
    std::cout << "\n=== Test 1: Single Match ===" << std::endl;
    allTestsPassed &= testMemoryAccess(manager, testData);
    
    // Test 2: Multi-Threading
    std::cout << "\n=== Test 2: Multi-Threading ===" << std::endl;
    allTestsPassed &= testMultiThreading(manager, testData);
    
    // Test 3: Große Partition
    std::cout << "\n=== Test 3: Large Partition ===" << std::endl;
    allTestsPassed &= testLargePartition(manager);
    
    // Test 4: Ungültige Partition
    std::cout << "\n=== Test 4: Invalid Partition ===" << std::endl;
    allTestsPassed &= testInvalidPartition(manager);
    
    // Speicher freigeben
    freeTestDataSet(testData);
    
    if (allTestsPassed) {
        std::cout << "\nAll tests completed successfully!" << std::endl;
        return 0;
    } else {
        std::cout << "\nSome tests failed!" << std::endl;
        return 1;
    }
}
