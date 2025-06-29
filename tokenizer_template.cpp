#include "Tokenization_mngr.h"

//we always read the original read-in string, from out dataSet-field, write them to our char-based indexing storages
extern "C" size_t {{FUNC_NAME}} ({{TEMP_TYPE_IN}} *line, {{TEMP_TYPE_OUT}} *out, Tokenization_mngr<{{TEMP_TYPES}}>* tkm)
{
    
{{FORMAT_CODE}}
    return 0;
}

