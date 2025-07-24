#define USE_STRUCTA_GUI
#include "structa.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;(void)hPrevInstance;(void)pCmdLine;(void)nCmdShow;

	AllocConsole();

	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);

	if (stInit() != ST_SUCCESS)
	{
		printf("Failed structa init\n");
	}

	StWindowCreateInfo window_create_info = {
		.title = "Structa Application",
		.width = 640,
		.height = 480
	};
	
	// Init systems

	StWindow window = { 0 };
	stCreateWindow(&window_create_info, &window);
	
	StRenderer renderer = { 0 };
	stCreateRenderer(&renderer);

	StGuiInitInfo gui_init_info = {0};
	stInitGuiSystem(&gui_init_info);

	while (!stWindowShouldClose(window))
	{
		stRender(renderer);

		stPollEvents();
	}

	stShutdownGuiSystem();
	stDestroyRenderer();
	stDestroyWindow();
	stShutdown();

	FreeConsole();
	return 0;
}