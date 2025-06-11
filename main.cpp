#include <iostream>
#include <filesystem>
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

    int start = clock();

    // 1. Datei-Objekte erzeugen
    File file1(files[0]);
    File file2(files[1]);
    File file3(files[2]);
    File file4(files[3]);

    // 2. Multi-Threaded Parsing für alle Datasets
    dataSet<char*>* dataSet1 = parser_mngr.parse_multithreaded<char*>(
        file1.data(), file1.size(), file1.line_count(), "%_,%V", maxThreads);

    dataSet<storage>* dataSet2 = parser_mngr.parse_multithreaded<storage>(
        file2.data(), file2.size(), file2.line_count(), "%_,%s,%f,%s,%s,%V", maxThreads);

    dataSet<match>* dataSetSol1 = parser_mngr.parse_multithreaded<match>(
        file3.data(), file3.size(), file3.line_count(), "%d,%d", maxThreads);

    dataSet<match>* dataSetSol2 = parser_mngr.parse_multithreaded<match>(
        file4.data(), file4.size(), file4.line_count(), "%d,%d", maxThreads);

    printf("time elapsed for reading Files: %ld ms\n", clock()-start);

    start = clock(); //reset clock

    printf("generating Blocks...\n");

    block_t* blocksDS1 = m_blocking_mngr->generateBlocks(dataSet1->size, maxThreads);
    block_t* blocksDS2 = m_blocking_mngr->generateBlocks(dataSet2->size, maxThreads);

    printf("generating matching...\n");

    match* matchesDS1 = m_matching_mngr->generateMatching(blocksDS1, figureOut, maxThreads);
    match* matchesDS2 = m_matching_mngr->generateMatching(blocksDS2, figureOut, maxThreads);

    printf("time elapsed for finding matches: %ld", clock()-start);

    float DS1EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS1, dataSetSol1->c_arr(), maxThreads);
    float DS2EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS2, dataSetSol2->c_arr(), maxThreads);
    
    printf("Evaluation of Duplicate Detection within DataSet1: %f\n", DS1EvaluationScore);
    printf("Evaluation of Duplicate Detection within DataSet2: %f\n", DS2EvaluationScore);

    return clock()-start;
}