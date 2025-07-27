#include "structa.h"
#include <stdio.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance; (void)hPrevInstance; (void)pCmdLine; (void)nCmdShow;
	AllocConsole();
	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);

	stInit();
	stCreateWindow("Structa Application", 640, 480);
	stCreateRenderer();

	stCreateTriangle();

	while (!stWindowShouldClose())
	{
		stRender();
		stPollEvents();
	}

	stDestroyTriangle();
	stDestroyRenderer();
	stDestroyWindow();
	stShutdown();

	FreeConsole();
	return 0;
}