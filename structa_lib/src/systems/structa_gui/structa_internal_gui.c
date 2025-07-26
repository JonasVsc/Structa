#include "structa_internal_gui.h"
#include "shared/structa_helpers.h"
#include "structa_gui.h"


static StGuiSystem_T g_system = { 0 };

static StResult structa_init_gui_resources(StGui gui);

StResult stInitGuiSystem(const StGuiInitInfo* init_info)
{
	const size_t total_size = sizeof(StGui_T);
	g_system.storage = (StGui_T*)calloc(1, total_size);
	g_system.size = total_size;

	g_system.ptr_gui = (StGui_T*)g_system.storage;

	StGui gui = g_system.ptr_gui;
	gui->device = init_info->device;

	if (structa_init_gui_resources(gui) != ST_SUCCESS)
		return ST_ERROR;

	return ST_SUCCESS;
}

void stShutdownGuiSystem()
{
	if (g_system.storage == NULL)
		return;

	StGui gui = structa_internal_get_gui_system_ptr();

	vkDestroyPipelineLayout(gui->device, gui->layout, NULL);
	vkDestroyPipeline(gui->device, gui->pipeline, NULL);

	free(g_system.storage = NULL);
	g_system.storage = NULL;
}

StGui structa_internal_get_gui_system_ptr()
{
	return g_system.ptr_gui;
}

StResult structa_init_gui_resources(StGui gui)
{
	gui->pipeline = structa_create_default_pipeline(gui->device, gui->surface_format, &gui->layout);

	if(gui->pipeline == VK_NULL_HANDLE)
		return ST_ERROR;
	return ST_SUCCESS;
}
