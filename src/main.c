#include <stdio.h>
#include "window.h"
#include "renderer.h"

int main(int argc, char** argv)
{
	printf("Hello, from Structa!\n");

#ifdef NDEBUG
	printf("Running release mode!\n");
#else
	printf("Running release mode!\n");
#endif

	StWindow window;
	StWindowCreateInfo windowCI = {
		.title = "teste",
		.width = 640,
		.height = 480
	};
	stCreateWindow(&windowCI, &window);

	StRenderer renderer;
	stCreateRenderer(&window, &renderer);

	while (!window.shouldClose)
	{
		stPoolEvents(&window);
		stRender();
	}

	stDestroyRenderer(&renderer);
	stDestroyWindow(&window);
	return 0;
}