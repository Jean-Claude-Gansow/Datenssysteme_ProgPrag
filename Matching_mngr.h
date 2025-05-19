#include "DataTypes.h"

#ifndef MATCHING_MANAGER
#define MATCHING_MANAGER

class Matching_mngr {

public:
    Matching_mngr(){}
    ~Matching_mngr(){}
public:
    match* generateMatching(block_t* , unsigned int len, unsigned int numThreads = 1); //function to create threads, collect and order their work
    match* identifyMatches(block_t , unsigned int len); //function to pass to threads, identifies Matches within a block
};

#endif