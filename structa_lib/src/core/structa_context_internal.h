#ifndef STRUCTA_CONTEXT_INTERNAL_H_
#define STRUCTA_CONTEXT_INTERNAL_H_ 1

#include "window/structa_window_internal.h"
#include "renderer/structa_renderer_internal.h"

typedef struct StContext_T* StContext;

typedef struct StContext_T {
	StWindow_T window;
	StRenderer_T renderer;
} StContext_T;

extern StContext StructaContext;

#endif // STRUCTA_CONTEXT_INTERNAL_H_