#include "gamebase.h"

#define MAX_ENTITIES 1024

typedef struct StEntitiesManager {
	StRenderable renderable[MAX_ENTITIES];
	uint32_t count;
} StEntitiesManager;

typedef struct StGamebaseContext {
	StEntitiesManager entities;
	StRenderer renderer;
} StGamebase;

static StGamebase gb = { 0 };

// =============================================================================
// Public API Implementation
// =============================================================================

void stInitGamebaseSubsystem(StRenderer renderer)
{
	gb.renderer = renderer;
}

StResult stCreateEntity(const StEntityCreateInfo* createInfo, EntityID* ID)
{
	if (createInfo == NULL)
	{
		return ST_ERROR_INCOMPLETE_CREATE_INFO;
	}

	*ID = gb.entities.count;

	/* == TODO: Use a Obj Loader API (ex: Assimp) =========================== */
	float vertices[] = {
		// position		// color
		 0.5f,  0.5f,	1.0f, 1.0f, 0.0f,
		 0.0f, -0.5f,	0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,	0.0f, 0.0f, 1.0f,
	};
	
	StRenderableCreateInfo renderableCI = {
		.transformCreateInfo = &createInfo->transformCreateInfo,
		.size = sizeof(vertices),
		.src = vertices,
	};
	// =============================================================================
	
	stCreateRenderable(gb.renderer, &renderableCI, &gb.entities.renderable[*ID]);

	++gb.entities.count;

	return ST_SUCCESS;
}

void stDestroyEntity(EntityID ID)
{

}
