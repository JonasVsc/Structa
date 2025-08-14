#include "structa_window.h"
#include "structa_internal.h"

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void structaCreateWindow(const char* title, uint32_t width, uint32_t height)
{
	StructaContext g = GStructaContext;

	HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
	const char CLASS_NAME[] = "Window Class";
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	g->window.width = width;
	g->window.height = height;
	g->window.handle = CreateWindow(CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		(int)g->window.width, (int)g->window.height, NULL, NULL, instance, NULL);

	ShowWindow(g->window.handle, SW_SHOWNORMAL);
}