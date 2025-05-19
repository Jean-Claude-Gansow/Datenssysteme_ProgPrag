//
// Created by Jean-Claude on 19.05.2025.
//
#include "DataTypes.h"

#ifndef DUPLICATEDETECTION_BLOCKING_MNGR_H
#define DUPLICATEDETECTION_BLOCKING_MNGR_H


class Blocking_mngr 
{
   


private:
    unsigned short blockSize;

public:
    Blocking_mngr();
    Blocking_mngr(unsigned int blockSize);
    ~Blocking_mngr();

public:
    block_t* generateBlocks(unsigned int len, unsigned int Threads); //generate block by generating and ordering blocklines within threads
    block_t generateBlock(unsigned int size, unsigned int start); //generate Block, calls the 
    pair* generateBlockLine(unsigned int end = 1); //generate matches for each id, run in threads

};


#endif //DUPLICATEDETECTION_BLOCKING_MNGR_H
