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


	// =============================================================================
	// Create Renderable
	// =============================================================================
	// TODO: Alloc memory for renderables

	// TEMPORARY: This will be replaced by a assets loader API
	float vertices[] = {
		// position		// color
		 0.5f,  0.5f,	1.0f, 1.0f, 0.0f,
		 0.0f, -0.5f,	0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,	0.0f, 0.0f, 1.0f,
	};

	StRenderableCreateInfo renderableCI = {
		.size = sizeof(vertices),
		.src = vertices
	};

	StRenderable renderable;
	ST_CHECK(stCreateRenderable(renderer, &renderableCI, &renderable));


	while (!stShouldClose(renderer))
	{
		stRender(renderer);
		stPollEvents(renderer);
	}

	stDestroyRenderer(renderer);

	return 0;
}