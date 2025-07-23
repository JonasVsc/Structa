#include "structa.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;(void)hPrevInstance;(void)pCmdLine;(void)nCmdShow;

#ifndef NDEBUG
	AllocConsole();

	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);
#endif // NDEBUG

	if (stInit() != ST_SUCCESS)
	{
		printf("Failed structa init\n");
	}

	StWindowCreateInfo window_create_info = {
		.title = "Structa Application",
		.width = 640,
		.height = 480
	};

	stCreateWindow(&window_create_info, NULL);
	stCreateRenderer(NULL);

	stDestroyRenderer();
	stDestroyWindow();
	stShutdown();
	return 0;
}