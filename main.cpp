#include <iostream>
#include <string.h>
#include <thread>
#include <time.h>

#include "constants.h"
#include "Evaluation_mngr.h"
#include "Blocking_mngr.h"
#include "Matching_mngr.h"
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

int main(int argc, char** argv)
{   
    char test[3] = "\"\n";
    printf("%hu\n",(unsigned short)((*test << 8) | *(test+1)));


    printf("Reading Dataset: Laptops from path: %s\n",files[0].c_str());
    printf("Reading Dataset: Storage from path: %s\n",files[1].c_str());
    printf("Reading Dataset: Laptops-Solution from path: %s\n",files[2].c_str());
    printf("Reading Dataset: Storage-Solution from path: %s\n",files[3].c_str());

    Blocking_mngr* m_blocking_mngr = new Blocking_mngr();
    Matching_mngr* m_matching_mngr = new Matching_mngr();
    Evaluation_mngr* m_evaluation_mngr = new Evaluation_mngr();
    
    unsigned int figureOut = 0; //unknown by now, filling that in later
    unsigned int maxThreads = std::thread::hardware_concurrency();
    maxThreads = maxThreads > 1 ? maxThreads : 1; //in case thread count failed set it to one thread

    Parser_mngr parser_mngr;

    int start_total = clock();

    // 1. Datei-Objekte erzeugen
    File file1(files[0]);
    File file2(files[1]);
    File file3(files[2],true);
    File file4(files[3],true);

    int start = clock();
    // 2. Multi-Threaded Parsing für alle Datasets
    dataSet<single_t>* dataSet1 = parser_mngr.parse_multithreaded<single_t>(file1.data(), file1.size(), file1.line_count(), "%_,%V", maxThreads);

    printf("Parsed %zu lines from file1:\n", dataSet1->size);
    print_Dataset(*dataSet1, "%_,%V");

    dataSet<storage>* dataSet2 = parser_mngr.parse_multithreaded<storage>(file2.data(), file2.size(), file2.line_count(), "%_,%s,%f,%s,%s,%V", maxThreads);

    printf("Parsed %zu lines from file2\n", dataSet2->size);
    print_Dataset(*dataSet2, "%_,%s,%f,%s,%s,%V");

    dataSet<match>* dataSetSol1 = parser_mngr.parse_multithreaded<match>(file3.data(), file3.size(), file3.line_count(), "%d,%d", maxThreads);
    
    printf("Parsed %zu lines from file3\n", dataSetSol1->size);
    print_Dataset(*dataSetSol1, "%d,%d");


    dataSet<match>* dataSetSol2 = parser_mngr.parse_multithreaded<match>(file4.data(), file4.size(), file4.line_count(), "%d,%d", maxThreads);
    
    printf("Parsed %zu lines from file4\n", dataSetSol2->size);
    print_Dataset(*dataSetSol2, "%d,%d");

    // Zu Beginn auswählbares Dataset zum Ausgeben
    // Einfach hier den gewünschten Pointer setzen:
    auto* printDataSet = dataSetSol1; // z.B. dataSet1, dataSet2, dataSetSol1, dataSetSol2

    // Anzahl der Zeilen, die ausgegeben werden sollen (z.B. 10)
    size_t printRows = 10;
    printRows = std::min(printRows, printDataSet->size);

   /* printf("Ausgabe des ausgewählten Datasets (%zu Zeilen):\n", printRows);
    for(int i = 0; i < printRows; ++i) 
    {
            printf("%d, %d\n", printDataSet->c_arr()[i][0], printDataSet->c_arr()[i][1]);
    }*/

    int elapsedParse = clock() - start;
    printf("time elapsed for reading Files: %.2f s\n", elapsedParse / (float)CLOCKS_PER_SEC);

    start = clock();

    printf("generating Blocks...\n");

    block_t* blocksDS1 = m_blocking_mngr->generateBlocks(dataSet1->size, maxThreads);
    block_t* blocksDS2 = m_blocking_mngr->generateBlocks(dataSet2->size, maxThreads);

    int elapsedBlock = clock() - start;
    printf("time elapsed for generating Blocks: %.2f s\n", elapsedBlock / (float)CLOCKS_PER_SEC);

    start = clock();

    match* matchesDS1 = m_matching_mngr->generateMatching(blocksDS1, figureOut, maxThreads);
    match* matchesDS2 = m_matching_mngr->generateMatching(blocksDS2, figureOut, maxThreads);

    int elapsedMatch = clock() - start;
    printf("time elapsed for matching: %.2f s\n", elapsedMatch / (float)CLOCKS_PER_SEC);

    start = clock();

    float DS1EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS1, dataSetSol1->c_arr(), maxThreads);
    float DS2EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS2, dataSetSol2->c_arr(), maxThreads);
    
    int elapsedEval = clock() - start;
    printf("time elapsed for evaluation: %.2f s\n", elapsedEval / (float)CLOCKS_PER_SEC);

    int elapsedTotal = clock() - start_total;
    printf("total time elapsed: %.2f s\n", elapsedTotal / (float)CLOCKS_PER_SEC);

    printf("Evaluation of Duplicate Detection within DataSet1: %f\n", DS1EvaluationScore);
    printf("Evaluation of Duplicate Detection within DataSet2: %f\n", DS2EvaluationScore);

    return 0;
}