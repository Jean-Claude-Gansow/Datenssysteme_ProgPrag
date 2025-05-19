#include "DataTypes.h"

//
// Created by Jean-Claude on 19.05.2025.
//

#ifndef DUPLICATEDETECTION_FILEINPUT_H
#define DUPLICATEDETECTION_FILEINPUT_H

dataSet* readCSV(const char* filename,const char* format)
{
    FILE* f = fopen(filename,"r");
    //implement logic to read csv according to format and return into dataset structure
    return nullptr;
}

template <typename t>
void readFormat(const char* filename, const char* format, t* buf)
{
    FILE* f = fopen(filename,"r");
    //implement logic to read file according to format, output in structure or buffer, trusting that size will fit 
}

#endif //DUPLICATEDETECTION_FILEINPUT_H
