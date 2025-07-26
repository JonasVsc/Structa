#include "app.h"

static StApplication g_app = { 0 };

void setup()
{
	if (stInit() != ST_SUCCESS)
	{
		printf("Failed structa init\n");
	}

	StWindowCreateInfo window_create_info = {
		.title = "Structa application",
		.width = 1280,
		.height = 720
	};

	stCreateWindow(&window_create_info, &g_app.window);
	stCreateRenderer(&g_app.renderer);

	stCreateTriangle2();

	StGuiInitInfo gui_init_info = {
		.device = structa_get_device(g_app.renderer),
	};

	stInitGuiSystem(&gui_init_info);
}

void run()
{
	while (!stWindowShouldClose(g_app.window))
	{
		stRender(g_app.renderer);
		stPollEvents();
	}
}

void cleanup()
{
	stDestroyTriangle2();
	stShutdownGuiSystem();
	stDestroyRenderer();
	stDestroyWindow();
	stShutdown();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance; (void)hPrevInstance; (void)pCmdLine; (void)nCmdShow;

	AllocConsole();

	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);

	setup();

	run();

	cleanup();

	FreeConsole();
	return 0;
}