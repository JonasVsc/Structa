#include "structa_gui.h"

static StGuiSystem_T g_system = { 0 };

StResult stInitGuiSystem(const StGuiInitInfo* init_info)
{
	const size_t total_size = sizeof(StGui_T);
	g_system.storage = calloc(1, total_size);
	g_system.size = total_size;
	return ST_SUCCESS;
}

void stShutdownGuiSystem()
{
	if (g_system.storage == NULL) 
		return;

	free(g_system.storage = NULL);
	g_system.storage = NULL;
}

