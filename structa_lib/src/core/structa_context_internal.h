#ifndef STRUCTA_CONTEXT_INTERNAL_H_
#define STRUCTA_CONTEXT_INTERNAL_H_ 1

#include "window/structa_window_internal.h"
#include "renderer/structa_renderer_internal.h"


#define MAX_MESHES 100


typedef struct StContext_T* StContext;

typedef struct StContext_T {
	StWindow_T window;
	StRenderer_T renderer;
	// RESOURCES
	StMesh_T meshes[MAX_MESHES];
	uint32_t mesh_capacity;
	uint32_t mesh_count;


} StContext_T;

extern StContext StructaContext;

#endif // STRUCTA_CONTEXT_INTERNAL_H_