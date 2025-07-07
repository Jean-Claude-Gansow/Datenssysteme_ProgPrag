//
// Created by Jean-Claude on 19.05.2025.
//

#include "DataTypes.h"

#ifndef DUPLICATEDETECTION_EVALUATION_MNGR_H
#define DUPLICATEDETECTION_EVALUATION_MNGR_H


class Evaluation_mngr 
{
public:
    size_t threshhold = 200;
public:
    Evaluation_mngr()
    {}
    
    ~Evaluation_mngr(){}

public:
    float evaluateMatches(dataSet<matching>* matches, dataSet<match>* Solution); // set class resources, before forking, so workers can use this
};


#endif //DUPLICATEDETECTION_EVALUATION_MNGR_H
