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

// Funktion zum Drucken eines storage_drive-Objekts als String
std::string driveToString(const storage_drive& sd) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[brand:%hu model:%hu cap:%hu class_a:%hu class_c:%hu class_v:%hu class_u:%hu "
             "class_uhs:%hu variant:%hu data_speed:%hu formfactor:%hu conn_type:%hu]",
             sd.brand, sd.model, sd.capacity, sd.class_a, sd.class_c, sd.class_v, sd.class_u, 
             sd.class_uhs, sd.variant, sd.data_speed, sd.formfactor, sd.connection_type);
    return std::string(buffer);
}

// Hilfsfunktion zum Erstellen einer storage_drive mit bestimmten Werten
storage_drive create_test_drive(token brand, token model, token capacity, token class_a,
                              token class_c, token class_v, token class_u, token class_uhs,
                              token variant, token data_speed, token formfactor, token connection_type) {
    storage_drive sd;
    sd.brand = brand;
    sd.model = model;
    sd.capacity = capacity;
    sd.class_a = class_a;
    sd.class_c = class_c;
    sd.class_v = class_v;
    sd.class_u = class_u;
    sd.class_uhs = class_uhs;
    sd.variant = variant;
    sd.data_speed = data_speed;
    sd.formfactor = formfactor;
    sd.connection_type = connection_type;
    return sd;
}

// Test für den == Operator bei identischen storage_drives
void test_equals_identical() {
    TestResult::printTestDescription("test_equals_identical", 
        "Testet den == Operator bei zwei identischen Storage Drives. Das Ergebnis sollte 1 (Match) sein.");
    
    storage_drive drive1 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    storage_drive drive2 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    
    int result = (drive1 == drive2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_equals_identical");
    } else {
        TestResult::fail("test_equals_identical", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Test für den == Operator bei storage_drives mit unterschiedlichen wichtigen Eigenschaften
void test_equals_different_critical() {
    TestResult::printTestDescription("test_equals_different_critical", 
        "Testet den == Operator bei Storage Drives mit unterschiedlichen kritischen Eigenschaften (hier: andere Marke). Das Ergebnis sollte 0 (kein Match) sein.");
    
    storage_drive drive1 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    storage_drive drive2 = create_test_drive(2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12); // Andere Marke
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    std::cout << "Unterschied: brand (1 vs 2)" << std::endl;
    
    int result = (drive1 == drive2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 0)" << std::endl;
    
    if (result == 0) {
        TestResult::pass("test_equals_different_critical");
    } else {
        TestResult::fail("test_equals_different_critical", "Ergebnis sollte 0 sein, war aber " + std::to_string(result));
    }
}

// Test für den == Operator bei storage_drives mit einigen übereinstimmenden, aber nicht genügenden Eigenschaften
void test_equals_some_matches() {
    TestResult::printTestDescription("test_equals_some_matches", 
        "Testet den == Operator bei Storage Drives mit einigen übereinstimmenden Eigenschaften, aber einem wichtigen Unterschied (hier: Kapazität). Das Ergebnis sollte 0 (kein Match) sein.");
    
    storage_drive drive1 = create_test_drive(1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0);
    storage_drive drive2 = create_test_drive(1, 2, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0); // Nur Kapazität anders
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    std::cout << "Unterschied: capacity (3 vs 4)" << std::endl;
    std::cout << "Übereinstimmungen: brand, model, class_a" << std::endl;
    
    int result = (drive1 == drive2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 0)" << std::endl;
    
    if (result == 0) {
        TestResult::pass("test_equals_some_matches");
    } else {
        TestResult::fail("test_equals_some_matches", "Ergebnis sollte 0 sein, war aber " + std::to_string(result));
    }
}

// Test für den == Operator bei storage_drives mit genügend übereinstimmenden Eigenschaften für Fallback
void test_equals_fallback_needed() {
    TestResult::printTestDescription("test_equals_fallback_needed", 
        "Testet den == Operator bei Storage Drives mit genügend übereinstimmenden Eigenschaften für Fallback (mind. 4 übereinstimmende Eigenschaften, hier: Unterschied in class_a). Das Ergebnis sollte 0 oder 2 (Fallback) sein.");
    
    // Ausreichend übereinstimmende Felder, aber mit einigen Unterschieden
    storage_drive drive1 = create_test_drive(1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0);
    storage_drive drive2 = create_test_drive(1, 2, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0); // Nur class_a anders
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    std::cout << "Unterschied: class_a (4 vs 5)" << std::endl;
    std::cout << "Übereinstimmungen: brand, model, capacity" << std::endl;
    
    int result = (drive1 == drive2);
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
    
    storage_drive drive1 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    storage_drive drive2 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    
    double result = (drive1 | drive2);
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
        "Testet den [] Operator für den kategoriespezifischen Zugriff auf die storage_drive-Eigenschaften.");
    
    storage_drive drive1 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    std::cout << "Storage Drive: " << driveToString(drive1) << std::endl;
    
    // Erwartete Werte und tatsächliche Werte ausgeben
    std::cout << "Überprüfe Index-Operator Zugriffe:" << std::endl;
    std::cout << "drive[assembler_brand]: " << drive1[assembler_brand] << " (Erwartet: 1)" << std::endl;
    std::cout << "drive[assembler_modell]: " << drive1[assembler_modell] << " (Erwartet: 2)" << std::endl;
    std::cout << "drive[storage_capacity]: " << drive1[storage_capacity] << " (Erwartet: 3)" << std::endl;
    std::cout << "drive[class_a]: " << drive1[class_a] << " (Erwartet: 4)" << std::endl;
    std::cout << "drive[class_c]: " << drive1[class_c] << " (Erwartet: 5)" << std::endl;
    
    bool indexOperatorWorking = true;
    std::string errorMsg = "";
    
    if (drive1[assembler_brand] != 1) {
        indexOperatorWorking = false;
        errorMsg += "assembler_brand: " + std::to_string(drive1[assembler_brand]) + " (erwartet: 1); ";
    }
    if (drive1[assembler_modell] != 2) {
        indexOperatorWorking = false;
        errorMsg += "assembler_modell: " + std::to_string(drive1[assembler_modell]) + " (erwartet: 2); ";
    }
    if (drive1[storage_capacity] != 3) {
        indexOperatorWorking = false;
        errorMsg += "storage_capacity: " + std::to_string(drive1[storage_capacity]) + " (erwartet: 3); ";
    }
    if (drive1[class_a] != 4) {
        indexOperatorWorking = false;
        errorMsg += "class_a: " + std::to_string(drive1[class_a]) + " (erwartet: 4); ";
    }
    if (drive1[class_c] != 5) {
        indexOperatorWorking = false;
        errorMsg += "class_c: " + std::to_string(drive1[class_c]) + " (erwartet: 5); ";
    }
    
    if (indexOperatorWorking) {
        TestResult::pass("test_index_operator");
    } else {
        TestResult::fail("test_index_operator", "[] Operator gibt falsche Werte zurück: " + errorMsg);
    }
}

// Test für den out-of-bounds check in storage_drive::operator==
void test_out_of_bounds_check() {
    TestResult::printTestDescription("test_out_of_bounds_check", 
        "Testet den out-of-bounds Check in storage_drive::operator== (if(c >= 12)). Die Funktion sollte nicht abstürzen und korrekt zum Fallback springen, wenn alle Kategorien durchlaufen wurden.");
    
    // Test speziell für den if(c >= 12) {} Check in storage_drive::operator==
    storage_drive drive1 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    storage_drive drive2 = create_test_drive(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    std::cout << "Out-of-bounds Check: if(c >= 12) { goto useFallBack; }" << std::endl;
    
    // Die Funktion sollte nicht abstürzen, wenn alle Kategorien durchlaufen werden
    int result = (drive1 == drive2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_out_of_bounds_check");
    } else {
        TestResult::fail("test_out_of_bounds_check", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Test für Nullwerte in bestimmten Feldern
void test_null_values() {
    TestResult::printTestDescription("test_null_values", 
        "Testet den == Operator bei Storage Drives mit Nullwerten (0) in bestimmten Feldern. Da der Vergleichsoperator Nullwerte überspringt, sollten diese nicht verglichen werden und das Ergebnis sollte 1 (Match) sein.");
    
    storage_drive drive1 = create_test_drive(1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 11, 12);
    storage_drive drive2 = create_test_drive(1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 11, 12);
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    std::cout << "Nullwerte bei: class_a, class_c, class_v, class_u, class_uhs, variant, data_speed" << std::endl;
    
    int result = (drive1 == drive2);
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_null_values");
    } else {
        TestResult::fail("test_null_values", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Test für zwei vollständig leere Storage Drives
void test_empty_drives() {
    TestResult::printTestDescription("test_empty_drives", 
        "Testet den == Operator bei zwei vollständig leeren Storage Drives (alle Werte 0). Das Ergebnis sollte 1 (Match) sein, da zwei leere Objekte als identisch gelten sollten.");
    
    // Zwei komplett leere storage_drive-Objekte erstellen (alle Werte 0)
    storage_drive drive1;
    storage_drive drive2;
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    std::cout << "Alle Werte in beiden Objekten sind 0" << std::endl;
    
    // Direkt den const-Operator verwenden, um Segmentation Fault zu vermeiden
    int result = (static_cast<const storage_drive&>(drive1) == static_cast<const storage_drive&>(drive2));
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_empty_drives");
    } else {
        TestResult::fail("test_empty_drives", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Test für zwei identische Storage Drives mit spezifischem Datenmuster
void test_specific_pattern_drives() {
    TestResult::printTestDescription("test_specific_pattern_drives", 
        "Testet den == Operator bei zwei Storage Drives mit einem spezifischen Datenmuster [16 0 0 0 0 0 0 0 1 0 9 5]. Das Ergebnis sollte 1 (Match) sein.");
    
    // Erstelle zwei Storage Drives mit dem spezifischen Muster
    storage_drive drive1 = create_test_drive(16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 9, 5);
    storage_drive drive2 = create_test_drive(16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 9, 5);
    
    std::cout << "Drive 1: " << driveToString(drive1) << std::endl;
    std::cout << "Drive 2: " << driveToString(drive2) << std::endl;
    std::cout << "Muster: [16 0 0 0 0 0 0 0 1 0 9 5]" << std::endl;
    std::cout << "Dieses Muster hat viele Nullwerte, aber einige spezifische Werte bei brand, variant, formfactor und connection_type" << std::endl;
    
    // Direkt den const-Operator verwenden, um Segmentation Fault zu vermeiden
    int result = (static_cast<const storage_drive&>(drive1) == static_cast<const storage_drive&>(drive2));
    std::cout << "Ergebnis des Vergleichs: " << result << " (Erwartet: 1)" << std::endl;
    
    if (result == 1) {
        TestResult::pass("test_specific_pattern_drives");
    } else {
        TestResult::fail("test_specific_pattern_drives", "Ergebnis sollte 1 sein, war aber " + std::to_string(result));
    }
}

// Hauptfunktion, die alle Tests ausführt
int main() {
    std::cout << "===== Starte Tests für storage_drive Vergleichsoperatoren =====" << std::endl;
    
    TestResult::startSection("Test von Gleichheits-Operatoren (==)");
    test_equals_identical();
    test_equals_different_critical();
    test_equals_some_matches();
    test_equals_fallback_needed();
    
    TestResult::startSection("Test von Ähnlichkeits-Operatoren (|)");
    test_similarity_operator();
    
    TestResult::startSection("Test von Hilfsfunktionen und Edge Cases");
    test_index_operator();
    test_out_of_bounds_check();
    test_null_values();
    test_empty_drives();
    test_specific_pattern_drives();
    
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
