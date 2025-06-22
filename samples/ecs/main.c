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
	// Initialize Subsystems
	// =============================================================================
	
	stInitGamebaseSubsystem(renderer);

	// =============================================================================
	// Create Entity
	// =============================================================================

	StEntityCreateInfo entityCI = {
		.transformCreateInfo = {
			.initialPosition = { 0 },
			.initialRotation = { 0 },
			.initialScale = { 1.0f, 1.0f, 1.0f }
		}
	};

	EntityID myEntity;
	stCreateEntity(&entityCI, &myEntity);


	// =============================================================================
	// Main loop
	// =============================================================================
	while (!stShouldClose(renderer))
	{
		stRender(renderer);
		stPollEvents(renderer);
	}

	stDestroyRenderer(renderer);

	return 0;
}