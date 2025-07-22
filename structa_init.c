#include "structa_internal.h"
#include "structa_core.h"

static StMemory g_memory = { 0 };

StResult stInitSystem()
{
	stAllocateMemory(&g_memory);

	if (g_memory == NULL) return ST_ERROR;

	return ST_SUCCESS;
}

void stShutdown()
{
	if (g_memory == NULL) return;

	free(g_memory);
	g_memory = NULL;
}
