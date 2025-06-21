#include <structa.h>
#include "utils/helper.h"

int main(int argc, char** argv)
{
	StWindowCreateInfo windowCI = {
		.title = "window test",
		.width = 640,
		.height = 480,
	};

	StRendererCreateInfo rendererCI = {
		.windowCreateInfo = &windowCI
	};

	StRenderer renderer;
	ST_CHECK(stCreateRenderer(&rendererCI, &renderer));

	while (!stShouldClose(renderer))
	{
		stRender(&renderer);
		stPollEvents(renderer);
	}

	stDestroyRenderer(renderer);

	return 0;
}