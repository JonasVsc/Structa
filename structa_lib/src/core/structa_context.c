#include "structa_context_internal.h"
#include "structa_context.h"

StContext StructaContext = NULL;

StResult stInit()
{
	StructaContext = calloc(1, sizeof(StContext_T));
	if (StructaContext == NULL)
		return ST_ERROR;

	return ST_SUCCESS;
}

void stShutdown()
{
	free(StructaContext);
	StructaContext = NULL;
}

