#include "structa.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)pCmdLine;
	(void)nCmdShow;

	StWindow window = { 0 };

	StWindowCreateInfo window_create_info = {
		.title = "Structa Application",
		.width = 640,
		.height = 480
	};

	stCreateWindow(&window_create_info, &window);


	return 0;
}