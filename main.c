#include "structa.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)pCmdLine;
	(void)nCmdShow;

	if (stInitSystem() != ST_SUCCESS)
	{
		OutputDebugString("Failed init structa");
	}


	stShutdown();
	return 0;
}