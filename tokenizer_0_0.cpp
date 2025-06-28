#include "Tokenization_mngr.h"

//we always read the original read-in string, from out dataSet-field, write them to our char-based indexing storages
extern "C" size_t tokenizer_0_0 (single_t *line, laptop **out, Tokenization_mngr<12,single_t,laptop>* tkm)
{
    
	tkm->filter_tokens((char*)(line->data[0]), out);
	printf("done Analyzing V field 0\n");

    return 0;
}

