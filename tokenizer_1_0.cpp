#include "Tokenization_mngr.h"

//we always read the original read-in string, from out dataSet-field, write them to our char-based indexing storages
extern "C" size_t tokenizer_1_0 (quintupel *line, storage_drive **out, Tokenization_mngr<4,quintupel,storage_drive>* tkm)
{
    
	tkm->filter_tokens((char*)(line->data[0]), out);
	tkm->filter_tokens((char*)(line->data[2]), out);
	tkm->filter_tokens((char*)(line->data[3]), out);
	tkm->filter_tokens((char*)(line->data[4]), out);

    return 0;
}

