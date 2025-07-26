#include "structa_internal.h"
#include "structa_core.h"

static StStructaSystem_T g_system = { 0 };

StResult stInit()
{
	// ALLOC SYSTEM STORAGE
	const size_t total_size = sizeof(StWindow_T) + sizeof(StRenderer_T);
	g_system.storage = calloc(1, total_size);
	g_system.size = total_size;

	// INIT SubSystem Pointers
	g_system.ptr_window = g_system.storage;
	g_system.ptr_renderer = (StRenderer_T*)((char*)g_system.storage + sizeof(StWindow_T));

	return ST_SUCCESS;
}

void stShutdown()
{
	if (g_system.storage == NULL)
		return;

	free(g_system.storage);
	g_system.storage = NULL;
}

StWindow structa_internal_window_ptr()
{
	return g_system.ptr_window;
}

StRenderer structa_internal_renderer_ptr()
{
	return g_system.ptr_renderer;
}


