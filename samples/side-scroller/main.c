#include <stdio.h>
#include "window.h"
#include "renderer.h"

#include "utils/helper.h"

#ifdef NDEBUG
#define ST_CHECK(x) x
#else
#define ST_CHECK(x)																										\
		do																													\
		{																													\
			StResult err = x;																								\
			if (err)																										\
			{																												\
				fprintf(stderr, "\033[38;2;255;128;128;4;5m Detected Structa error: %s\033[0m", stStringResult(err));		\
				abort();																									\
			}																												\
		} while (0)
#endif


int main(int argc, char** argv)
{
	printf("Hello, from side-scroller!\n");

#ifdef NDEBUG
	printf("Running release mode!\n");
#else
	printf("Running debug mode!\n");
#endif

	StWindow window;
	StWindowCreateInfo windowCI = {
		.title = "teste",
		.width = 640,
		.height = 480
	};
	ST_CHECK(stCreateWindow(&windowCI, &window));

	StRenderer renderer;
	ST_CHECK(stCreateRenderer(&window, &renderer));

	while (!window.shouldClose)
	{
		stPoolEvents(&window);
		stRender();
	}

	stDestroyRenderer(&renderer);
	stDestroyWindow(&window);
	return 0;
}