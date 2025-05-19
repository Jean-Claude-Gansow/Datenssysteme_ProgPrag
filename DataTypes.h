#ifndef DUPLICATE_DETECTION_DATATYPES
#define DUPLICATE_DETECTION_DATATYPES


typedef struct dataSet_t
{
    char** Cells {0};
    unsigned int len {0};
    unsigned int width {0};
} dataSet;

typedef struct matching_t
{
    match* matches;
    unsigned int size;

    match& operator[](unsigned int index) 
    {
        return matches[index];
    }

    const match& operator[](unsigned int index) const 
    {
        return matches[index];
    }
} matching;


template <unsigned int N,typename t>
struct tuple_t {
    t data[N];

    // Optional: Zugriffsfunktion
    t& operator[](std::size_t index) 
    {
        return data[index];
    }

    const t& operator[](std::size_t index) const 
    {
        return data[index];
    }
};


typedef tuple_t<2,int> pair,match;
typedef pair** block_t;

#endif