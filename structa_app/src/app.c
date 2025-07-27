#include "structa.h"
#include "structa_gui.h"

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

	StContextInfo ctx_info = stGetContextInfo();

	StGuiInitInfo gui_init_info = {
		.device = ctx_info.device,
		.queue = ctx_info.queue,
		.queue_family = ctx_info.queue_family,
		.format = ctx_info.format
	};

	stInitGuiSystem(&gui_init_info);
	
	MeshId triangle_mesh = stLoadMesh();

	while (!stWindowShouldClose())
	{
		stBeginFrame();

		stDraw(triangle_mesh);

		stEndFrame();
		stPollEvents();
	}

	stFreeMeshes();
	stShutdownGuiSystem();
	stDestroyRenderer();
	stDestroyWindow();
	stShutdown();

	FreeConsole();
	return 0;
}