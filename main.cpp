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
#include "debug_utils.h"  // Include debugging utilities first
#include "Evaluation_mngr.h"
#include "partitioning_mngr.h"
#include "Matching_mngr.h"
#include "Tokenization_mngr.h"
#include "Parser_mngr.h"
#include "FileInput.h"
#include "DataTypes.h"

std::string files[] = 
{
    "../data/Z1.csv",
    "../data/Z2.csv",
    "../data/ZY1.csv",
    "../data/ZY2.csv"
};

// Die Funktion ist jetzt in debug_utils.h definiert

int main(int argc, char** argv)
{   
    // Print debug configuration information
    print_debug_configuration();
    
    char test[3] = "\"\n";
    printf("%hu\n",(unsigned short)((*test << 8) | *(test+1)));

    printf("Reading Dataset: Laptops from path: %s\n",files[0].c_str());
    printf("Reading Dataset: Storage from path: %s\n",files[1].c_str());
    printf("Reading Dataset: Laptops-Solution from path: %s\n",files[2].c_str());
    printf("Reading Dataset: Storage-Solution from path: %s\n",files[3].c_str());

    Tokenization_mngr<12, single_t, laptop> *m_Laptop_tokenization_mngr = new Tokenization_mngr<12, single_t, laptop>({"12","single_t","laptop"});
    Tokenization_mngr<4, quintupel, storage_drive> *m_Storage_tokenization_mngr = new Tokenization_mngr<4, quintupel, storage_drive>({"4","quintupel","storage_drive"});
    Partitioning_mngr<single_t,laptop,12>* m_partitioning_laptop_mngr = new Partitioning_mngr<single_t,laptop,12>(1000);
    Partitioning_mngr<quintupel,storage_drive,4> *m_partitioning_storage_mngr = new Partitioning_mngr<quintupel,storage_drive,4>(1000);
    Matching_mngr<laptop>* m_matching_laptop_mngr = new Matching_mngr<laptop>();
    Matching_mngr<storage_drive> *m_matching_storage_mngr = new Matching_mngr<storage_drive>();
    Evaluation_mngr* m_evaluation_mngr = new Evaluation_mngr();
    
    unsigned int figureOut = 0; //unknown by now, filling that in later
    unsigned int maxThreads = std::thread::hardware_concurrency();
    maxThreads = maxThreads > 0 ? maxThreads : 1; //in case thread count failed set it to one thread

    Parser_mngr parser_mngr;

    int start_total = clock();

    // 1. Datei-Objekte erzeugen
    File file1(files[0],true);
    File file2(files[1],true);
    File file3(files[2],true);
    File file4(files[3],true);

    //assembler_brand: 0
    //assembler_modell: 1
    //ram_capacity: 2
    //rom_capacity: 3
    //

    std::vector<category> laptop_partition_hierarchy = {assembler_brand, assembler_modell, cpu_brand, ram_capacity};
    std::vector<category> storage_partition_hierarchy = {assembler_brand, assembler_modell, rom_capacity};
    //TODO: Listenbäume aufbauen. Format: Token;Token;Token;...Token\n -> index = line
    //TODO: Listen zuusammenführen -> Klassenindices können erst dann korrekt gebaut werden.
    m_Laptop_tokenization_mngr->loadTokenList("../data/laptop_marken.tokenz",assembler_brand); //laptop brand
    m_Laptop_tokenization_mngr->loadTokenList("../data/laptop_modelle.tokenz", assembler_modell); //laptop modell

    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_marken.tokenz", cpu_brand); //cpu brand ++ inference tokens for cpu modells
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_amd.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_intel.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_ibm.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_modelle_qualcomm.tokenz",cpu_fam,cpu_brand); //cpu familly (i3,i5,i7...)
    //m_Laptop_tokenization_mngr->loadTokenList("../data/cpu_series.tokenz", cpu_series); //cpu series (10400kf,14100K...)
    m_Laptop_tokenization_mngr->loadTokenList("../data/gpu_marken.tokenz", gpu_brand);//gpu brand 
    //m_Laptop_tokenization_mngr->loadTokenList("../data/gpu_modelle.tokenz", gpu_fam);// gpu familly (gt,gtx,rtx,radeon ...)
    //m_Laptop_tokenization_mngr->loadTokenList("../data/gpu_series.tokenz", gpu_series);// gpu series (no clue yet)
    m_Laptop_tokenization_mngr->loadTokenList("../data/ram_size.tokenz",ram_capacity);//
    m_Laptop_tokenization_mngr->loadTokenList("../data/rom_size.tokenz",rom_capacity);

    m_Storage_tokenization_mngr->loadTokenList("../data/festplatten_marken.tokenz",assembler_brand);
    m_Storage_tokenization_mngr->loadTokenList("../data/festplatten_modelle.tokenz", assembler_modell);
    m_Storage_tokenization_mngr->loadTokenList("../data/rom_size_festplatten.tokenz",rom_capacity);
    m_Storage_tokenization_mngr->loadTokenList("../data/festplatten_schnittstellen.tokenz")

    int start = clock();

    // 2. Multi-Threaded Parsing für alle Datasets
    dataSet<single_t>* dataSet1 = parser_mngr.parse_multithreaded<single_t>(file1.data(), file1.size(), file1.line_count(), "%_,%V", maxThreads);
    printf("Parsed %zu lines from file1:\n", dataSet1->size);
    print_Dataset(*dataSet1, "%_,%V");

    dataSet<quintupel>* dataSet2 = parser_mngr.parse_multithreaded<quintupel>(file2.data(), file2.size(), file2.line_count(), "%_,%s,%f,%s,%s,%V", maxThreads);
    printf("Parsed %zu lines from file2\n", dataSet2->size);
    print_Dataset(*dataSet2, "%_,%s,%f,%s,%s,%V");

    //dataSet<match>* dataSetSol1 = parser_mngr.parse_multithreaded<match>(file3.data(), file3.size(), file3.line_count(), "%d,%d", maxThreads);
    //printf("Parsed %zu lines from file3\n", dataSetSol1->size);
    //print_Dataset(*dataSetSol1, "%d,%d");

    //dataSet<match>* dataSetSol2 = parser_mngr.parse_multithreaded<match>(file4.data(), file4.size(), file4.line_count(), "%d,%d", maxThreads);
    //printf("Parsed %zu lines from file4\n", dataSetSol2->size);
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

    tokenized_laptops->print();
    tokenized_storage->print();

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

    int elapsedBlock = clock() - start;
    printf("time elapsed for generating Blocks: %.2f s\n", elapsedBlock / (float)CLOCKS_PER_SEC);

    start = clock();

    printf("Starting duplicate detection within partitions...\n");
    // Use simpler matching approach for better debugging
    dataSet<matching>* matchesDS1 = m_matching_laptop_mngr->identify_matches(laptop_partitions, maxThreads);
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
    
    printf("\nFound %zu potential duplicates in laptop dataset (%zu partitions)\n", 
           total_laptop_matches, matchesDS1->size);
    printf("Found %zu potential duplicates in storage dataset (%zu partitions)\n", 
           total_storage_matches, matchesDS2->size);

    int elapsedMatch = clock() - start;
    printf("time elapsed for matching: %.2f s\n", elapsedMatch / (float)CLOCKS_PER_SEC);

    start = clock();

    //float DS1EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS1, dataSetSol1->c_arr(), maxThreads);
    //float DS2EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS2, dataSetSol2->c_arr(), maxThreads);
    
    int elapsedEval = clock() - start;
    printf("time elapsed for evaluation: %.2f s\n", elapsedEval / (float)CLOCKS_PER_SEC);
    printf("\n\n==================================================================================================================================\n\n");

    int elapsedTotal = clock() - start_total;
    printf("total time elapsed: %.2f s\n", elapsedTotal / (float)CLOCKS_PER_SEC);
    printf("\n\n==================================================================================================================================\n\n");

    //printf("Evaluation of Duplicate Detection within DataSet1: %f\n", DS1EvaluationScore);
    //printf("Evaluation of Duplicate Detection within DataSet2: %f\n", DS2EvaluationScore);

    // Aufräumen - Speicher freigeben
    if (matchesDS1 && matchesDS1->data) delete matchesDS1->data;
    
    if (matchesDS2 && matchesDS2->data) delete matchesDS2->data;
    delete matchesDS1;
    delete matchesDS2;

    return 0;
}