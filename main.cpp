#include <iostream>
#include <filesystem>
#include <string.h>
#include <thread>

#include "time.h"
#include "Evaluation_mngr.h"
#include "Blocking_mngr.h"
#include "Matching_mngr.h"
#include "FileInput.h"
#include "DataTypes.h"


int main(char** argv, int argc)
{
    if(argc < 2)
    {
        printf("Missing Dataset argument");
        exit(1);
    }

    dataSet DS1;
    dataSet DS2;

    const char* pathDS1 = strcat(argv[2],"/data/X1.cvs");
    const char* pathDS2 = strcat(argv[2],"/data/X2.cvs");
    const char* pathS1 = strcat(argv[2],"/data/XY1.cvs");
    const char* pathS2 = strcat(argv[2],"/data/XY2.cvs");
    
    Blocking_mngr* m_blocking_mngr = new Blocking_mngr();
    Matching_mngr* m_matching_mngr = new Matching_mngr();
    Evaluation_mngr* m_evaluation_mngr = new Evaluation_mngr();
    
    unsigned int figureOut = 0; //unknown by now, filling that in later
    unsigned int maxThreads = std::thread::hardware_concurrency();
    maxThreads = maxThreads > 1 ? maxThreads : 1; //in case thread count failed set it to one thread

    int start = clock();

    //run cvs file input, track time in ms
    dataSet* dataSet1 = readCSV("pathDS1","%_,%s");
    dataSet* dataSet2 = readCSV("pathDS2","%_,%s,%d");

    match* dataSetSol1; 
    readFormat<match>("pathS1","%d,%d",dataSetSol1);
    match* dataSetSol2;
    readFormat<match>("pathS2","%d,%d",dataSetSol2);

    printf("time elapsed for reading Files: %d", clock()-start);

    start = clock(); //reset clock

    auto blocksDS1 = m_blocking_mngr->generateBlocks(dataSet1->len,maxThreads);
    auto blocksDS2 = m_blocking_mngr->generateBlocks(dataSet2->len,maxThreads);

    match* matchesDS1 = m_matching_mngr->generateMatching(blocksDS1,figureOut,maxThreads);
    match* matchesDS2 = m_matching_mngr->generateMatching(blocksDS2,figureOut,maxThreads);

    printf("time elapsed for finding matches: %d", clock()-start);

    float DS1EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS1, dataSetSol1, maxThreads);
    float DS2EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS2, dataSetSol2, maxThreads);
    
    printf("Evaluation of Duplicate Detection within DataSet1: %u\n", DS1EvaluationScore);
    printf("Evaluation of Duplicate Detection within DataSet2: %u\n", DS2EvaluationScore);

    return clock()-start;
}