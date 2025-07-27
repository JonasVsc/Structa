#include "structa_context_internal.h"
#include "structa_context.h"

StContext StructaContext = NULL;

StResult stInit()
{
	size_t total_alloc = sizeof(StContext_T);
	StructaContext = calloc(1, total_alloc);
	if (StructaContext == NULL)
		return ST_ERROR;

	StructaContext->mesh_capacity = MAX_MESHES;

	return ST_SUCCESS;
}

void stShutdown()
{
	free(StructaContext);
	StructaContext = NULL;
}

StContextInfo stGetContextInfo()
{
	StContextInfo context_info = {
		.device = StructaContext->renderer.device,
		.queue = StructaContext->renderer.graphics_queue,
		.queue_family = StructaContext->renderer.graphics_queue_family,
		.format = StructaContext->renderer.swapchain_format.format
	};

	return context_info;
}