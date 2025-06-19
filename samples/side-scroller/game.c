#include "game.h"

#include "components.h"

typedef struct AppState {
	StWindow window;
	StRenderer renderer;
} AppState;

static AppState app = {0};

void run()
{
	StWindowCreateInfo windowCI = {
		.width = 640,
		.height = 480,
		.title = "Basic Entity"
	};

	stCreateWindow(&windowCI, &app.window);
	stCreateRenderer(&app.window, &app.renderer);

	// Create Scene
	StSceneCreateInfo sceneCI = {
		.initialCapacity = 10
	};

	StScene scene = { 0 };
	stCreateScene(&sceneCI, &scene);

	// Create Renderable
	float vertices[] = {
		-0.5f, -0.5f, 0.2f, 0.4f, 1.0f,
		 0.0f,  0.5f, 0.2f, 0.4f, 1.0f,
		 0.5f, -0.5f, 0.2f, 0.4f, 1.0f,
	};

	StRenderableCreateInfo renderableCI = {
		.size = sizeof(vertices),
		.src = vertices,
	};

	StRenderable renderable = { 0 };
	renderable.vertexCount = 3;
	renderable.draw = 1;
	stCreateRenderable(&renderableCI, &renderable);
	stSceneAddRenderable(&scene, &renderable);

	stSetScene(&scene);

	while (!app.window.shouldClose)
	{
		stPoolEvents(&app.window);
		stRender();
	}

	stDestroyWindow(&app.window);
	stDestroyRenderer(&app.renderer);
}
