#include "structa_core.h"
#include "structa_internal.h"

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

StResult stCreateWindow(const  StWindowCreateInfo* create_info, StWindow window)
{
	StWindow internal_window = structa_internal_window_ptr();

	if (create_info == NULL) return ST_ERROR;

	internal_window->title = create_info->title;
	internal_window->width = create_info->width;
	internal_window->height = create_info->height;

	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	const char CLASS_NAME[] = "Window Class";
	WNDCLASS windowClass = {0};
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = CLASS_NAME;
	RegisterClass(&windowClass);

	internal_window->handle = CreateWindow(CLASS_NAME, internal_window->title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, internal_window->width, internal_window->height, NULL, NULL, hInstance, NULL);
	ShowWindow(internal_window->handle, SW_SHOWNORMAL);

	window = internal_window;
	return ST_SUCCESS;
}

void stDestroyWindow()
{
	StWindow internal_window = structa_internal_window_ptr();
	DestroyWindow(internal_window->handle);
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

