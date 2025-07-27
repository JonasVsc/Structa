#include "core/structa_context_internal.h"
#include "structa_window_internal.h"
#include "structa_window.h"


LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

StResult stCreateWindow(const char* title, uint32_t width, uint32_t height)
{
	if (StructaContext == NULL)
		return ST_ERROR;

	StWindow window = &StructaContext->window;

	window->title = title;
	window->width = width;
	window->height = height;
	window->close = false;

	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	const char CLASS_NAME[] = "Window Class";
	WNDCLASS windowClass = { 0 };
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = CLASS_NAME;
	RegisterClass(&windowClass);

	window->handle = CreateWindow(CLASS_NAME, window->title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window->width, window->height, NULL, NULL, hInstance, NULL);
	ShowWindow(window->handle, SW_SHOWNORMAL);
	return ST_SUCCESS;
}

void stDestroyWindow()
{
	DestroyWindow(StructaContext->window.handle);
}

bool stWindowShouldClose()
{
	return StructaContext->window.close;
}

void stPollEvents()
{
	MSG msg = { 0 };
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		StructaContext->window.close = true;
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

