#include "structa_core.h"

typedef struct StWindow_T {
	const char* title;
	int height;
	int width;
} StWindow_T;

StResult stCreateWindow(const  StWindowCreateInfo* create_info, StWindow* window)
{
	if (!create_info || !window) return ST_ERROR;

	*window = (StWindow_T*)calloc(1, sizeof(StWindow_T));

	if (*window == NULL) return ST_ERROR;

	(*window)->title = create_info->title;
	(*window)->width = create_info->width;
	(*window)->height = create_info->height;

	return ST_SUCCESS;
}

void stDestroyWindow(StWindow window)
{
	if (window == NULL) return;
	free(window);
}
