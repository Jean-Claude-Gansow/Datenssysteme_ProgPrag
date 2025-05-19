//
// Created by Jean-Claude on 19.05.2025.
//

#include "DataTypes.h"

#ifndef DUPLICATEDETECTION_EVALUATION_MNGR_H
#define DUPLICATEDETECTION_EVALUATION_MNGR_H


class Evaluation_mngr 
{
private:
    match* found;
    match* solution;

public:
    Evaluation_mngr();
    ~Evaluation_mngr();

public:
    float evaluateMatches(match* matches = 0, match* Solution = 0, unsigned int MaxThreads = 1);//set class resources, before forking, so workers can use this
    float evaluateMatches(unsigned int start = 0, unsigned int end = 1);
};


#endif //DUPLICATEDETECTION_EVALUATION_MNGR_H
