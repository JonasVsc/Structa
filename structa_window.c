#include "structa_core.h"

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct StWindow_T {
	const char* title;
	uint32_t height;
	uint32_t width;
	HWND handle;
} StWindow_T;

StResult stCreateWindow(const  StWindowCreateInfo* create_info, StWindow* window)
{
	if (!create_info || !window) return ST_ERROR;

	*window = (StWindow_T*)calloc(1, sizeof(StWindow_T));

	if (*window == NULL) return ST_ERROR;

	(*window)->title = create_info->title;
	(*window)->width = create_info->width;
	(*window)->height = create_info->height;

	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	const char CLASS_NAME[] = "Window Class";
	WNDCLASS windowClass = {0};
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = CLASS_NAME;
	RegisterClass(&windowClass);

	window = CreateWindow(CLASS_NAME, (*window)->title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (*window)->width, (*window)->height, NULL, NULL, hInstance, NULL);
	ShowWindow(window, SW_SHOWNORMAL);

	return ST_SUCCESS;
}

void stDestroyWindow(StWindow window)
{
	if (window == NULL) return;
	DestroyWindow(window->handle);

	free(window);
	window = NULL;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

