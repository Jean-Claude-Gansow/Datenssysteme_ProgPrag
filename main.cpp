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

dataSet DS1;
dataSet DS2;
char* files[] = 
{
    "../data/Z1.cvs",
    "../data/Z2.cvs",
    "../data/ZY1.cvs",
    "../data/ZY2.cvs"
};

int main(int argc, char** argv)
{
    
    
    printf("Reading Dataset: Laptops from path: %s\n",files[0]);
    printf("Reading Dataset: Storage from path: %s\n",files[1]);
    printf("Reading Dataset: Laptops-Solution from path: %s\n",files[2]);
    printf("Reading Dataset: Storage-Solution from path: %s\n",files[3]);

    Blocking_mngr* m_blocking_mngr = new Blocking_mngr();
    Matching_mngr* m_matching_mngr = new Matching_mngr();
    Evaluation_mngr* m_evaluation_mngr = new Evaluation_mngr();
    
    unsigned int figureOut = 0; //unknown by now, filling that in later
    unsigned int maxThreads = std::thread::hardware_concurrency();
    maxThreads = maxThreads > 1 ? maxThreads : 1; //in case thread count failed set it to one thread

    int start = clock();

    //run cvs file input, track time in ms
    dataSet* dataSet1 = readCSV(files[0],"%_,%s");
    dataSet* dataSet2 = readCSV(files[1],"%_,%s,%d");

    match* dataSetSol1; 
    readFormat<match>(files[2],"%d,%d",dataSetSol1);
    match* dataSetSol2;
    readFormat<match>(files[3],"%d,%d",dataSetSol2);

    printf("time elapsed for reading Files: %ld ms\n", clock()-start);

    start = clock(); //reset clock

    printf("generating Blocks...\n");

    block_t* blocksDS1 = m_blocking_mngr->generateBlocks(dataSet1->len,maxThreads); //fails because dataSet::len isnt filled ATMs.
    block_t* blocksDS2 = m_blocking_mngr->generateBlocks(dataSet2->len,maxThreads);

    printf("generating matching...\n");

    match* matchesDS1 = m_matching_mngr->generateMatching(blocksDS1,figureOut,maxThreads);
    match* matchesDS2 = m_matching_mngr->generateMatching(blocksDS2,figureOut,maxThreads);

    printf("time elapsed for finding matches: %ld", clock()-start);

    float DS1EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS1, dataSetSol1, maxThreads);
    float DS2EvaluationScore = m_evaluation_mngr->evaluateMatches(matchesDS2, dataSetSol2, maxThreads);
    
    printf("Evaluation of Duplicate Detection within DataSet1: %f\n", DS1EvaluationScore);
    printf("Evaluation of Duplicate Detection within DataSet2: %f\n", DS2EvaluationScore);

    return clock()-start;
}