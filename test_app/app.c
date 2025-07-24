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
		.width = 640,
		.height = 480
	};

	stCreateWindow(&window_create_info, &g_app.window);
	stCreateRenderer(&g_app.renderer);
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