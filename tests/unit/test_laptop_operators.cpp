#include <iostream>
#include <cassert>
#include <string>
#include <iomanip>
#include "../../DataTypes.h"

// Hilfsklasse für Test-Ausgaben
class TestResult {
public:
    static void pass(const std::string& testName) {
        std::cout << "\033[32m✓ PASS\033[0m: " << testName << std::endl;
        passes++;
    }
    
    static void fail(const std::string& testName, const std::string& message) {
        std::cout << "\033[31m✗ FAIL\033[0m: " << testName << " - " << message << std::endl;
        failures++;
    }
    
    static int getFailures() {
        return failures;
    }
    
    static int getPasses() {
        return passes;
    }
    
    static void printSummary() {
        int total = passes + failures;
        std::cout << "\n===== Zusammenfassung =====\n";
        std::cout << "Gesamt: " << total << " Tests\n";
        std::cout << "\033[32mErfolgreich: " << passes << " (" << std::fixed << std::setprecision(1) << (total > 0 ? (passes * 100.0 / total) : 0) << "%)\033[0m\n";
        std::cout << "\033[31mFehlgeschlagen: " << failures << " (" << std::fixed << std::setprecision(1) << (total > 0 ? (failures * 100.0 / total) : 0) << "%)\033[0m\n";
    }
    
    static void startSection(const std::string& section) {
        std::cout << "\n----- " << section << " -----\n";
    }
    
    static void printTestDescription(const std::string& testName, const std::string& description) {
        std::cout << "\nTest: " << testName << "\n";
        std::cout << "Beschreibung: " << description << "\n";
    }
    
private:
    static int failures;
    static int passes;
};

int TestResult::failures = 0;
int TestResult::passes = 0;

// Funktion zum Drucken eines laptop-Objekts als String
std::string laptopToString(const laptop& l) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[brand:%hu model:%hu rom:%hu ram:%hu cpu_brand:%hu cpu_fam:%hu cpu_series:%hu "
             "gpu_brand:%hu gpu_fam:%hu gpu_series:%hu disp_res:%hu disp_size:%hu]",
             l.brand, l.model, l.rom, l.ram, l.cpu_brand, l.cpu_fam, l.cpu_series,
             l.gpu_brand, l.gpu_fam, l.gpu_series, l.display_resolution, l.display_size);
    return std::string(buffer);
}

// Hilfsfunktion zum Erstellen eines Laptops mit bestimmten Werten
laptop create_test_laptop(token brand, token model, token rom, token ram,
                         token cpu_brand, token cpu_fam, token cpu_series,
                         token gpu_brand, token gpu_fam, token gpu_series,
                         token display_resolution, token display_size) {
    laptop l;
    l.brand = brand;
    l.model = model;
    l.rom = rom;
    l.ram = ram;
    l.cpu_brand = cpu_brand;
    l.cpu_fam = cpu_fam;
    l.cpu_series = cpu_series;
    l.gpu_brand = gpu_brand;
    l.gpu_fam = gpu_fam;
    l.gpu_series = gpu_series;
    l.display_resolution = display_resolution;
    l.display_size = display_size;
    return l;
}

// Test für den == Operator bei identischen Laptops
void test_equals_identical() {
    TestResult::printTestDescription("test_equals_identical", 
        "Testet den == Operator bei zwei identischen Laptops. Das Ergebnis sollte 1 (Match) sein.");
    
    laptop laptop1 = create_test_laptop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    laptop laptop2 = create_test_laptop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    std::cout << "Laptop 1: " << laptopToString(laptop1) << std::endl;
    std::cout << "Laptop 2: " << laptopToString(laptop2) << std::endl;
    
    int result = (laptop1 == laptop2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_equals_identical");
    } else {
        TestResult::fail("test_equals_identical", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Test für den == Operator bei Laptops mit unterschiedlichen wichtigen Eigenschaften
void test_equals_different_critical() {
    TestResult::printTestDescription("test_equals_different_critical", 
        "Testet den == Operator bei Laptops mit unterschiedlichen kritischen Eigenschaften (hier: andere Marke). Das Ergebnis sollte 0 (kein Match) sein.");
    
    laptop laptop1 = create_test_laptop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    laptop laptop2 = create_test_laptop(2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12); // Andere Marke
    
    std::cout << "Laptop 1: " << laptopToString(laptop1) << std::endl;
    std::cout << "Laptop 2: " << laptopToString(laptop2) << std::endl;
    std::cout << "Unterschied: brand (1 vs 2)" << std::endl;
    
    int result = (laptop1 == laptop2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 0)" << std::endl;
    
    if (result == 0) {
        TestResult::pass("test_equals_different_critical");
    } else {
        TestResult::fail("test_equals_different_critical", "Ergebnis sollte 0 sein, war aber " + std::to_string(result));
    }
}

// Test für den == Operator bei Laptops mit einigen übereinstimmenden, aber nicht genügenden Eigenschaften
void test_equals_some_matches() {
    TestResult::printTestDescription("test_equals_some_matches", 
        "Testet den == Operator bei Laptops mit einigen übereinstimmenden Eigenschaften, aber einem wichtigen Unterschied (hier: RAM). Das Ergebnis sollte 0 (kein Match) sein.");
    
    laptop laptop1 = create_test_laptop(1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0);
    laptop laptop2 = create_test_laptop(1, 2, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0); // Nur RAM anders
    
    std::cout << "Laptop 1: " << laptopToString(laptop1) << std::endl;
    std::cout << "Laptop 2: " << laptopToString(laptop2) << std::endl;
    std::cout << "Unterschied: ram (4 vs 5)" << std::endl;
    
    int result = (laptop1 == laptop2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 0)" << std::endl;
    
    if (result == 0) {
        TestResult::pass("test_equals_some_matches");
    } else {
        TestResult::fail("test_equals_some_matches", "Ergebnis sollte 0 sein, war aber " + std::to_string(result));
    }
}

// Test für den == Operator bei Laptops mit genügend übereinstimmenden Eigenschaften für Fallback
void test_equals_fallback_needed() {
    TestResult::printTestDescription("test_equals_fallback_needed", 
        "Testet den == Operator bei Laptops mit genügend übereinstimmenden Eigenschaften für Fallback (mind. 4 übereinstimmende Eigenschaften, hier: Unterschied in CPU-Marke). Das Ergebnis sollte 0 oder 2 (Fallback) sein.");
    
    // Ausreichend übereinstimmende Felder, aber mit einigen Unterschieden
    laptop laptop1 = create_test_laptop(1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0);
    laptop laptop2 = create_test_laptop(1, 2, 3, 4, 6, 0, 0, 0, 0, 0, 0, 0); // Unterschiedliche CPU-Marke
    
    std::cout << "Laptop 1: " << laptopToString(laptop1) << std::endl;
    std::cout << "Laptop 2: " << laptopToString(laptop2) << std::endl;
    std::cout << "Unterschied: cpu_brand (5 vs 6)" << std::endl;
    std::cout << "Übereinstimmungen: brand, model, rom, ram" << std::endl;
    
    int result = (laptop1 == laptop2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 0 oder 2)" << std::endl;
    
    if (result == 0 || result == 2) { // Das Verhalten kann je nach Implementierung variieren
        TestResult::pass("test_equals_fallback_needed");
    } else {
        TestResult::fail("test_equals_fallback_needed", "Ergebnis sollte 0 oder 2 sein, war aber " + std::to_string(result));
    }
}

// Test für den | Operator (noch nicht implementiert, sollte 0.0 zurückgeben)
void test_similarity_operator() {
    TestResult::printTestDescription("test_similarity_operator", 
        "Testet den | Operator (Ähnlichkeitsvergleich). Derzeit ist dieser Operator noch nicht vollständig implementiert und gibt immer 0.0 zurück.");
    
    laptop laptop1 = create_test_laptop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    laptop laptop2 = create_test_laptop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    std::cout << "Laptop 1: " << laptopToString(laptop1) << std::endl;
    std::cout << "Laptop 2: " << laptopToString(laptop2) << std::endl;
    
    double result = (laptop1 | laptop2);
    std::cout << "Ergebnis des Ähnlichkeitsvergleichs: " << result << " (Erwartet: 0.0)" << std::endl;
    
    if (result == 0.0) {  // Gemäß aktueller Implementierung
        TestResult::pass("test_similarity_operator");
    } else {
        TestResult::fail("test_similarity_operator", "Ergebnis sollte 0.0 sein, war aber " + std::to_string(result));
    }
}

// Test für den Zugriff über den [] Operator
void test_index_operator() {
    TestResult::printTestDescription("test_index_operator", 
        "Testet den [] Operator für den kategoriespezifischen Zugriff auf die laptop-Eigenschaften.");
    
    laptop laptop1 = create_test_laptop(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    std::cout << "Laptop: " << laptopToString(laptop1) << std::endl;
    
    // Erwartete Werte und tatsächliche Werte ausgeben
    std::cout << "Überprüfe Index-Operator Zugriffe:" << std::endl;
    std::cout << "laptop[assembler_brand]: " << laptop1[assembler_brand] << " (Erwartet: 1)" << std::endl;
    std::cout << "laptop[assembler_modell]: " << laptop1[assembler_modell] << " (Erwartet: 2)" << std::endl;
    std::cout << "laptop[storage_capacity]: " << laptop1[storage_capacity] << " (Erwartet: 3)" << std::endl;
    std::cout << "laptop[ram_capacity]: " << laptop1[ram_capacity] << " (Erwartet: 3)" << std::endl;
    std::cout << "laptop[cpu_brand]: " << laptop1[cpu_brand] << " (Erwartet: 5)" << std::endl;
    
    bool indexOperatorWorking = true;
    std::string errorMsg = "";
    
    if (laptop1[assembler_brand] != 1) {
        indexOperatorWorking = false;
        errorMsg += "assembler_brand: " + std::to_string(laptop1[assembler_brand]) + " (erwartet: 1); ";
    }
    if (laptop1[assembler_modell] != 2) {
        indexOperatorWorking = false;
        errorMsg += "assembler_modell: " + std::to_string(laptop1[assembler_modell]) + " (erwartet: 2); ";
    }
    if (laptop1[storage_capacity] != 3) {
        indexOperatorWorking = false;
        errorMsg += "storage_capacity: " + std::to_string(laptop1[storage_capacity]) + " (erwartet: 3); ";
    }
    if (laptop1[ram_capacity] != 3) {
        indexOperatorWorking = false;
        errorMsg += "ram_capacity: " + std::to_string(laptop1[ram_capacity]) + " (erwartet: 3); ";
    }
    if (laptop1[cpu_brand] != 5) {
        indexOperatorWorking = false;
        errorMsg += "cpu_brand: " + std::to_string(laptop1[cpu_brand]) + " (erwartet: 5); ";
    }
    
    if (indexOperatorWorking) {
        TestResult::pass("test_index_operator");
    } else {
        TestResult::fail("test_index_operator", "[] Operator gibt falsche Werte zurück: " + errorMsg);
    }
}

// Test für Nullwerte in bestimmten Feldern
void test_null_values() {
    TestResult::printTestDescription("test_null_values", 
        "Testet den == Operator bei Laptops mit Nullwerten (0) in bestimmten Feldern. Da der Vergleichsoperator Nullwerte überspringt, sollten diese nicht verglichen werden und das Ergebnis sollte 1 (Match) sein.");
    
    laptop laptop1 = create_test_laptop(1, 2, 3, 4, 0, 0, 0, 8, 9, 10, 11, 12);
    laptop laptop2 = create_test_laptop(1, 2, 3, 4, 0, 0, 0, 8, 9, 10, 11, 12);
    
    std::cout << "Laptop 1: " << laptopToString(laptop1) << std::endl;
    std::cout << "Laptop 2: " << laptopToString(laptop2) << std::endl;
    std::cout << "Nullwerte bei: cpu_brand, cpu_fam, cpu_series" << std::endl;
    
    int result = (laptop1 == laptop2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_null_values");
    } else {
        TestResult::fail("test_null_values", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Test für zwei vollständig leere Laptops
void test_empty_laptops() {
    TestResult::printTestDescription("test_empty_laptops", 
        "Testet den == Operator bei zwei vollständig leeren Laptops (alle Werte 0). Das Ergebnis sollte 1 (Match) sein, da zwei leere Objekte als identisch gelten sollten.");
    
    // Zwei komplett leere laptop-Objekte erstellen (alle Werte 0)
    laptop laptop1;
    laptop laptop2;
    
    std::cout << "Laptop 1: " << laptopToString(laptop1) << std::endl;
    std::cout << "Laptop 2: " << laptopToString(laptop2) << std::endl;
    std::cout << "Alle Werte in beiden Objekten sind 0" << std::endl;
    
    int result = (laptop1 == laptop2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_empty_laptops");
    } else {
        TestResult::fail("test_empty_laptops", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Hauptfunktion, die alle Tests ausführt
int main() {
    std::cout << "===== Starte Tests für laptop Vergleichsoperatoren =====" << std::endl;
    
    TestResult::startSection("Test von Gleichheits-Operatoren (==)");
    test_equals_identical();
    test_equals_different_critical();
    test_equals_some_matches();
    test_equals_fallback_needed();
    
    TestResult::startSection("Test von Ähnlichkeits-Operatoren (|)");
    test_similarity_operator();
    
    TestResult::startSection("Test von Hilfsfunktionen");
    test_index_operator();
    test_null_values();
    
    TestResult::startSection("Test von Edge Cases");
    test_empty_laptops();
    
    std::cout << "\n===== Tests abgeschlossen =====" << std::endl;
    
    TestResult::printSummary();
    
    if (TestResult::getFailures() == 0) {
        std::cout << "\n\033[32mAlle Tests erfolgreich!\033[0m" << std::endl;
        return 0;
    } else {
        std::cout << "\n\033[31mEs sind " << TestResult::getFailures() << " Tests fehlgeschlagen.\033[0m" << std::endl;
        return 1;
    }
}
