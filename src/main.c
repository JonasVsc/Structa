#include <stdio.h>
#include "window.h"
#include "renderer.h"

int main(int argc, char** argv)
{
	printf("Hello, from Structa!\n");

#ifdef NDEBUG
	printf("Running release mode!\n");
#else
	printf("Running debug mode!\n");
#endif

	copyFile("window.dll", "window_temporary.dll");
	void *w = windowLoader("window_temporary.dll");
	void *r = rendererLoader("renderer.dll");

	StWindow window;
	StWindowCreateInfo windowCI = {
		.title = "teste",
		.width = 640,
		.height = 480
	};
	stCreateWindow(&windowCI, &window);

	StRenderer renderer;
	stCreateRenderer(&window, &renderer);

	uint64_t lastWrite = 0;
	getFileInformation("window_temporary.dll", &lastWrite);

	while (!window.shouldClose)
	{
		uint64_t lastWrite2 = 0;
		getFileInformation("window.dll", &lastWrite2);
		
		if(compareFileTime(lastWrite, lastWrite2) != 0)
		{
			windowUnloader(w);
			w = windowLoader("window_temporary.dll");
		}

		stPoolEvents(&window);
		stRender();
	}

	stDestroyRenderer(&renderer);
	stDestroyWindow(&window);

	rendererUnloader(r);
	windowUnloader(w);

	return 0;
}