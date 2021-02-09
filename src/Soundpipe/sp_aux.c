#include <string.h>
#include "sp.h"

int sp_auxdata_alloc(sp_auxdata *aux, size_t size)
{
    aux->ptr = malloc(size);
    aux->size = size;
    memset(aux->ptr, 0, size);
    return SP_OK;
}

int sp_auxdata_free(sp_auxdata *aux)
{
    free(aux->ptr);
    return SP_OK;
}

