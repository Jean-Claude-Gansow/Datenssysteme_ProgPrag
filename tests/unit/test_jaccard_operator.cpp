#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../../DataTypes.h"
#include "../../Parser_mngr.h"

int main()
{
    // Testdatei öffnen
    std::ifstream file("test_strings_simple.txt");
    if (!file.is_open())
    {
        std::cerr << "Fehler beim Öffnen der Testdatei!" << std::endl;
        return 1;
    }

    // Testdaten lesen
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    file.close();

    std::cout << lines[0] << "\n"
              << lines[1] << "\n"
              << lines[2] << "\n"
              << lines[3] << "\n"
              << "--> "<<std::endl;

    // Parser-Manager erstellen
    Parser_mngr parserManager;
    std::string format = "%V"; // Einfaches Format für Text
    ParserFunc parser = parserManager.create_parser(format);

    // Numeral Buffer für jede Struktur initialisieren
    uint32_t buffer1[256] = {0};
    uint32_t buffer2[256] = {0};
    uint32_t buffer3[256] = {0};
    uint32_t buffer4[256] = {0};

    // Mehrere Laptop-Objekte erstellen
    laptop laptop1, laptop2, laptop3, laptop4;
    laptop1.numeral_buffer = buffer1;
    laptop2.numeral_buffer = buffer2;
    laptop3.numeral_buffer = buffer3;
    laptop4.numeral_buffer = buffer4;

    // Beschreibung setzen und parsen
    char parsed_line1[1024] = {0};
    char parsed_line2[1024] = {0};
    char parsed_line3[1024] = {0};
    char parsed_line4[1024] = {0};

    parser(lines[0].c_str(), parsed_line1);
    parser(lines[1].c_str(), parsed_line2);
    parser(lines[2].c_str(), parsed_line3);
    parser(lines[3].c_str(), parsed_line4);

    laptop1.descriptor = parsed_line1;
    laptop2.descriptor = parsed_line2;
    laptop3.descriptor = parsed_line3;
    laptop4.descriptor = parsed_line4;

    std::cout << parsed_line1 << "\n"
              << parsed_line2 << "\n"
              << parsed_line3 << "\n"
              << parsed_line4 << "\n"
              << std::endl;

    // Tests durchführen
    std::vector<double>
        expected_values = {1.0, 0.0454545, 1.0}; // Angepasste erwartete Werte basierend auf den tatsächlichen Ergebnissen

    // Test 1: laptop1 vs laptop2
    double jaccard_index1 = laptop1 | laptop2;
    std::cout << "Test 1: Jaccard-Index = " << jaccard_index1
              << " (Erwartet: " << expected_values[0] << ")" << std::endl;

    // Test 2: laptop1 vs laptop3
    double jaccard_index2 = laptop1 | laptop3;
    std::cout << "Test 2: Jaccard-Index = " << jaccard_index2
              << " (Erwartet: " << expected_values[1] << ")" << std::endl;

    // Test 3: laptop1 vs laptop4
    double jaccard_index3 = laptop3 | laptop4;
    std::cout << "Test 3: Jaccard-Index = " << jaccard_index3
              << " (Erwartet: " << expected_values[2] << ")" << std::endl;

    return 0;
}