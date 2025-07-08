#include <iostream>
#include <string.h>
#include <thread>
#include <time.h>

// Uncomment one of these to enable different debug levels
// Limit_DEBUG_OUTPUT ist in debug_utils.h definiert
#define DEBUG_LEVEL 1  // Basic debugging (matching & partitioning only)
// #define DEBUG_LEVEL 2  // Detailed debugging (+ tokenization & detailed matching)
// #define DEBUG_LEVEL 3  // Full debugging (very verbose, + memory & full matching)

#include "constants.h"
#include "Evaluation_mngr.h"
#include "partitioning_mngr.h"
#include "Matching_mngr.h"
#include "Tokenization_mngr.h"
#include "Parser_mngr.h"
#include "FileInput.h"
#include "DataTypes.h"

std::string files[] =
{
    "../data/Test_Datasets/Laptop_Test-Datasets/laptop_4k.csv",
    "../data/Test_Datasets/Storage_Test-Datasets/storage_4k.csv",
    "../data/Test_Datasets/Laptop_Test-Datasets/laptop_4k_loesungen.csv",
    "../data/Test_Datasets/Storage_Test-Datasets/storage_4k_loesungen.csv"
};

/*std::string files[] =
{
    "../data/Z1.csv",
    "../data/Z2.csv",
    "../data/ZY1.csv",
    "../data/ZY2.csv"
};*/

// Die Funktion ist jetzt in debug_utils.h definiert

int main(int argc, char** argv)
{   
    // Print debug configuration information
    printf("Reading Dataset: Laptops from path: %s\n",files[0].c_str());
    printf("Reading Dataset: Storage from path: %s\n",files[1].c_str());
    printf("Reading Dataset: Laptops-Solution from path: %s\n",files[2].c_str());
    printf("Reading Dataset: Storage-Solution from path: %s\n",files[3].c_str());

    Tokenization_mngr<12, single_t, laptop> *m_Laptop_tokenization_mngr = new Tokenization_mngr<12, single_t, laptop>({"12","single_t","laptop"});
    Tokenization_mngr<12, quintupel, storage_drive> *m_Storage_tokenization_mngr = new Tokenization_mngr<12, quintupel, storage_drive>({"12","quintupel","storage_drive"});
    Partitioning_mngr<single_t,laptop,12>* m_partitioning_laptop_mngr = new Partitioning_mngr<single_t,laptop,12>();
    Partitioning_mngr<quintupel,storage_drive,12> *m_partitioning_storage_mngr = new Partitioning_mngr<quintupel,storage_drive,12>();
    Evaluation_mngr* m_evaluation_mngr = new Evaluation_mngr();
    
    unsigned int figureOut = 0; //unknown by now, filling that in later
    unsigned int maxThreads = std::thread::hardware_concurrency();
    maxThreads = maxThreads > 0 ? maxThreads : 1; //in case thread count failed set it to one thread

    Parser_mngr parser_mngr;

    int start_total = clock();

    // 1. Datei-Objekte erzeugen
    File file1(files[0]);
    File file2(files[1]);
    File file3(files[2],true);
    File file4(files[3],true);

    //assembler_brand: 0
    //assembler_modell: 1
    //ram_capacity: 2
    //rom_capacity: 3
    //

    std::vector<category> laptop_partition_hierarchy = {assembler_brand};
    std::vector<category> storage_partition_hierarchy = {assembler_brand};
    //TODO: Listenbäume aufbauen. Format: Token;Token;Token;...Token\n -> index = line
    //TODO: Listen zuusammenführen -> Klassenindices können erst dann korrekt gebaut werden.
    m_Laptop_tokenization_mngr->loadTokenList("../data/laptop_marken.tokenz",assembler_brand); //laptop brand
    m_Laptop_tokenization_mngr->loadTokenList("../data/acer_laptop_modelle.tokenz", assembler_modell,assembler_brand); //laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/asus_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/dell_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/fujitsu_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/gigabyte_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/hp_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/huawei_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/lenovo_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/lg_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/microsoft_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/msi_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/packard-bell_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/panasonic_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/samsung_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell
    m_Laptop_tokenization_mngr->loadTokenList("../data/sony_laptop_modelle.tokenz", assembler_modell, assembler_brand); // laptop modell//----
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_marken.tokenz", cpu_brand); //cpu brand ++ inference tokens for cpu modells
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_amd.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_intel.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_ibm.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_qualcomm.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/gpu_marken.tokenz", gpu_brand);//gpu brand 
    m_Laptop_tokenization_mngr->loadTokenList("../data/gpu_modelle_amd.tokenz", gpu_fam,gpu_brand);
    m_Laptop_tokenization_mngr->loadTokenList("../data/gpu_modelle_intel.tokenz", gpu_fam, gpu_brand);
    m_Laptop_tokenization_mngr->loadTokenList("../data/gpu_modelle_nvidia.tokenz", gpu_fam, gpu_brand);
    m_Laptop_tokenization_mngr->loadTokenList("../data/laptop_ram_size.tokenz",ram_capacity);//
    m_Laptop_tokenization_mngr->loadTokenList("../data/laptop_rom_size.tokenz",rom_capacity);
    m_Laptop_tokenization_mngr->loadTokenList("../data/laptop_display_resolutions.tokenz", display_resolution); //
    m_Laptop_tokenization_mngr->loadTokenList("../data/laptop_display_size.tokenz", display_size);

    m_Storage_tokenization_mngr->loadTokenList("../data/storage_marken.tokenz",assembler_brand);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_modelle.tokenz", assembler_modell);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_capacity.tokenz",storage_capacity);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_a_klassen.tokenz", class_a);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_c_klassen.tokenz", class_c);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_v_klassen.tokenz", class_v);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_u_klassen.tokenz", class_u);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_uhs_klassen.tokenz", class_uhs);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_varianten.tokenz", variant);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_daten_geschwindigkeiten.tokenz", data_speed);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_formfaktoren.tokenz", formfactor);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_pcie_schnittstellen.tokenz", connection_type);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_sata_schnittstellen.tokenz", connection_type);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_usb_schnittstellen.tokenz", connection_type);
    m_Storage_tokenization_mngr->loadTokenList("../data/storage_extra_schnittstellen.tokenz", connection_type);

    //m_Storage_tokenization_mngr->loadTokenList("../data/festplatten_schnittstellen.tokenz")

    int start = clock();

    // 2. Multi-Threaded Parsing für alle Datasets
    dataSet<single_t>* dataSet1 = parser_mngr.parse_multithreaded<single_t>(file1.data(), file1.size(), file1.line_count(), "%_,%V", maxThreads);
    printf("Parsed %zu lines from file1:\n", dataSet1->size);
    //print_Dataset(*dataSet1, "%_,%V");

    dataSet<quintupel>* dataSet2 = parser_mngr.parse_multithreaded<quintupel>(file2.data(), file2.size(), file2.line_count(), "%_,%s,%f,%s,%s,%V", maxThreads);
    printf("Parsed %zu lines from file2\n", dataSet2->size);
    //print_Dataset(*dataSet2, "%_,%s,%f,%s,%s,%V");

    dataSet<match>* dataSetSol1 = parser_mngr.parse_multithreaded<match>(file3.data(), file3.size(), file3.line_count(), "%d,%d", maxThreads);
    printf("Parsed %zu lines from file3\n", dataSetSol1->size);
    //print_Dataset(*dataSetSol1, "%d,%d");

    dataSet<match>* dataSetSol2 = parser_mngr.parse_multithreaded<match>(file4.data(), file4.size(), file4.line_count(), "%d,%d", maxThreads);
    printf("Parsed %zu lines from file4\n", dataSetSol2->size);
    //print_Dataset(*dataSetSol2, "%d,%d");

    int elapsedParse = clock() - start;
    printf("time elapsed for reading Files: %.2f s\n", elapsedParse / (float)CLOCKS_PER_SEC);
    printf("\n\n================================================================================================================================\n\n");
    start = clock();
    printf("tokenizing Data...\n");

    dataSet<laptop> *tokenized_laptops = m_Laptop_tokenization_mngr->tokenize_multithreaded(dataSet1, "%_,%V", maxThreads);
    dataSet<storage_drive> *tokenized_storage = m_Storage_tokenization_mngr->tokenize_multithreaded(dataSet2, "%_,%s,%f,%s,%s,%V", maxThreads);

    printf("tokenized dataset laptops: size: %zu\n",tokenized_laptops->size);
    printf("tokenized dataset storage: size: %zu\n", tokenized_storage->size);

    //tokenized_laptops->print();
    //tokenized_storage->print();

    int elapsedTokenize = clock() - start;
    printf("\n\n==================================================================================================================================\n\n");
    printf("time elapsed for tokenizing Datasets: %.2f s\n", elapsedParse / (float)CLOCKS_PER_SEC);
    printf("generating Partitions...\n");

    dataSet<partition> *laptop_partitions = m_partitioning_laptop_mngr->create_partitions(tokenized_laptops, m_Laptop_tokenization_mngr, laptop_partition_hierarchy);
    dataSet<partition> *storage_partitions = m_partitioning_storage_mngr->create_partitions(tokenized_storage, m_Storage_tokenization_mngr, storage_partition_hierarchy);
    printf("Generated %zu partitions for laptops\n", laptop_partitions->size);
    printf("Generated %zu partitions for storage drives\n", storage_partitions->size);
    
    // Print partition sizes for debugging
    printf("\nLaptop partition sizes:\n");
    size_t total_elements = 0;
    for (size_t i = 0; i < laptop_partitions->size; i++) {
        printf("Partition %zu: %zu elements\n", i, laptop_partitions->data[i].size);
        total_elements += laptop_partitions->data[i].size;
    }
    printf("Total elements across all laptop partitions: %zu\n\n", total_elements);
    
    printf("\nStorage partition sizes:\n");
    total_elements = 0;
    for (size_t i = 0; i < storage_partitions->size; i++) {
        printf("Partition %zu: %zu elements\n", i, storage_partitions->data[i].size);
        total_elements += storage_partitions->data[i].size;
    }
    printf("Total elements across all storage partitions: %zu\n\n", total_elements);
    //print_partitions_field(*laptop_partitions,0);
    //print_partitions_field(*storage_partitions, 0);

    printf("\n\n==================================================================================================================================\n\n");

    int elapsedBlock = clock() - start;
    printf("time elapsed for generating Blocks: %.2f s\n", elapsedBlock / (float)CLOCKS_PER_SEC);

    printf("\n\n==================================================================================================================================\n\n");

    start = clock();

    Matching_mngr<laptop> *m_matching_laptop_mngr = new Matching_mngr<laptop>(dataSet1->size);
    Matching_mngr<storage_drive> *m_matching_storage_mngr = new Matching_mngr<storage_drive>(dataSet2->size);

    printf("Starting duplicate detection within partitions...\n");
    // Use simpler matching approach for better debugging
    dataSet<matching>* matchesDS1 = m_matching_laptop_mngr->identify_matches(laptop_partitions, maxThreads);
    printf("Starting searching for duplicates of storage devices.\n");
    dataSet<matching> *matchesDS2 = m_matching_storage_mngr->identify_matches(storage_partitions, maxThreads);

    // Count actual matches (pairs)
    size_t total_laptop_matches = 0;
    size_t total_storage_matches = 0;
    
    printf("\nDetailed match counts:\n");
    
    printf("Laptop partitions matches:\n");
    for (size_t i = 0; i < matchesDS1->size; i++) {
        printf("Partition %zu: %zu matches\n", i, matchesDS1->data[i].size);
        total_laptop_matches += matchesDS1->data[i].size;
    }
    
    printf("Storage partitions matches:\n");
    for (size_t i = 0; i < matchesDS2->size; i++) {
        printf("Partition %zu: %zu matches\n", i, matchesDS2->data[i].size);
        total_storage_matches += matchesDS2->data[i].size;
    }
    
   
    printf("\n-----------------------------------------------------------------------------------------------------------------------------\\nn");

    printf("\nDetailed Laptop Matches (Format: <id1,id2>):\n");
    for (size_t i = 0; i < matchesDS2->size; i++)
    {
        if (matchesDS1->data[i].size > 0)
        {
            printf("Partition %zu Matches:\n", i);
            for (size_t j = 0; j < matchesDS1->data[i].size; j++)
            {
                printf("  <%zu,%zu>\n",
                       static_cast<size_t>(matchesDS1->data[i].matches[j].data[0]),
                       static_cast<size_t>(matchesDS1->data[i].matches[j].data[1]));
            }
        }
    }

    printf("\n-----------------------------------------------------------------------------------------------------------------------------\n\n");

    // Detaillierte Ausgabe der Storage-Drive Matches im Format <id1,id2>
    printf("\nDetailed Storage Drive Matches (Format: <id1,id2>):\n");
    for (size_t i = 0; i < matchesDS2->size; i++) {
        if (matchesDS2->data[i].size > 0) {
            printf("Partition %zu Matches:\n", i);
            for (size_t j = 0; j < matchesDS2->data[i].size; j++) {
                printf("  <%zu,%zu>\n", 
                    static_cast<size_t>(matchesDS2->data[i].matches[j].data[0]),
                    static_cast<size_t>(matchesDS2->data[i].matches[j].data[1]));
            }
        }
    }

    printf("\nFound %zu potential duplicates in laptop dataset (%zu partitions)\n", total_laptop_matches, matchesDS1->size);
    printf("Found %zu potential duplicates in storage dataset (%zu partitions)\n", total_storage_matches, matchesDS2->size);

    int elapsedMatch = clock() - start;
    printf("time elapsed for matching: %.2f s\n", elapsedMatch / (float)CLOCKS_PER_SEC);

    start = clock();

    float DS1EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS1, dataSetSol1);
    float DS2EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS2, dataSetSol2);
    
    int elapsedEval = clock() - start;
    printf("time elapsed for evaluation: %.2f s\n", elapsedEval / (float)CLOCKS_PER_SEC);
    printf("\n\n==================================================================================================================================\n\n");

    int elapsedTotal = clock() - start_total;
    printf("total time elapsed: %.2f s\n", elapsedTotal / (float)CLOCKS_PER_SEC);
    printf("\n\n==================================================================================================================================\n\n");

    //printf("Evaluation of Duplicate Detection within DataSet1: %f\n", DS1EvaluationScore);
    //printf("Evaluation of Duplicate Detection within DataSet2: %f\n", DS2EvaluationScore);

    // Aufräumen - Speicher freigeben
    // Matches aufräumen
    {
        if (matchesDS1) {
            if (matchesDS1->data) {
                for (size_t i = 0; i < matchesDS1->size; i++) {
                    if (matchesDS1->data[i].matches) {
                        delete[] matchesDS1->data[i].matches;
                    }
                }
                delete[] matchesDS1->data;
            }
            delete matchesDS1;
        }

        if (matchesDS2) {
            if (matchesDS2->data) {
                for (size_t i = 0; i < matchesDS2->size; i++) {
                    if (matchesDS2->data[i].matches) {
                        delete[] matchesDS2->data[i].matches;
                    }
                }
                delete[] matchesDS2->data;
            }
            delete matchesDS2;
        }

        // Partitionen aufräumen
        if (laptop_partitions) {
            if (laptop_partitions->data) {
                for (size_t i = 0; i < laptop_partitions->size; i++) {
                    if (laptop_partitions->data[i].data) {
                        delete[] laptop_partitions->data[i].data;
                    }
                }
                delete[] laptop_partitions->data;
            }
            delete laptop_partitions;
        }

        if (storage_partitions) {
            if (storage_partitions->data) {
                for (size_t i = 0; i < storage_partitions->size; i++) {
                    if (storage_partitions->data[i].data) {
                        delete[] storage_partitions->data[i].data;
                    }
                }
                delete[] storage_partitions->data;
            }
            delete storage_partitions;
        }

        // Tokenisierte Daten aufräumen
        if (tokenized_laptops) {
            if (tokenized_laptops->data) {
                delete[] tokenized_laptops->data;
            }
            delete tokenized_laptops;
        }

        if (tokenized_storage) {
            if (tokenized_storage->data) {
                delete[] tokenized_storage->data;
            }
            delete tokenized_storage;
        }

        // Eingabedaten aufräumen
        if (dataSet1) {
            if (dataSet1->data) {
                delete[] dataSet1->data;
            }
            delete dataSet1;
        }

        if (dataSet2) {
            if (dataSet2->data) {
                delete[] dataSet2->data;
            }
            delete dataSet2;
        }

        // Manager aufräumen
        delete m_Laptop_tokenization_mngr;
        delete m_Storage_tokenization_mngr;
        delete m_partitioning_laptop_mngr;
        delete m_partitioning_storage_mngr;
        delete m_matching_laptop_mngr;
        delete m_matching_storage_mngr;
        delete m_evaluation_mngr;
    }
    return 0;
}