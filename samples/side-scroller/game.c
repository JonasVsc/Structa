#include "game.h"

#include "components.h"

#define MAX_ENTITIES 100000

typedef struct Entity {
	TransformComponent transform;
	MeshComponent mesh;
} Entity;

typedef struct Scene {
	Entity entities[MAX_ENTITIES];
	uint32_t entityCount;
} Scene;

typedef struct AppState {
	StWindow window;
	StRenderer renderer;
	Scene scene;
} AppState;

static AppState app = { 0 };

static StResult sceneAddEntity(Scene* scene, Entity* entity);
static StResult setScene(Scene* sc);

void run()
{
	StWindowCreateInfo windowCI = {
		.width = 640,
		.height = 480,
		.title = "Basic Entity"
	};

	stCreateWindow(&windowCI, &app.window);
	stCreateRenderer(&app.window, &app.renderer);

	// Load Mesh
	float vertices[] = {
		 0.5f,  0.5f,	1.0f, 0.0f, 0.0f,
		 0.0f, -0.5f,	0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,	0.0f, 0.0f, 1.0f,
	};

	StMeshCreateInfo meshCI = {
		.size = sizeof(vertices),
		.src = vertices,
		.vertexCount = 3
	};

	uint32_t myMesh;
	stLoadMesh(&meshCI, &myMesh);

	// Create Entity
	Entity entity = {
		.transform = {
			.position = {0},
			.rotation = {0},
			.scale = {1.0f, 1.0f, 1.0f}
		},
		.mesh = {
			.isVisible = true,
			.meshID = myMesh
		}
	};

	// Add Entity to a Scene
	sceneAddEntity(&app.scene, &entity);

	// Set Scene to Render
	setScene(&app.scene);


	while (!app.window.shouldClose)
	{
		stPoolEvents(&app.window);
		stRender();
	}

	stDestroyWindow(&app.window);
	stDestroyRenderer(&app.renderer);
}

static StResult sceneAddEntity(Scene* scene, Entity* entity)
{
	if (scene->entityCount >= MAX_ENTITIES)
		return ST_ERROR;

	scene->entities[scene->entityCount] = *entity;
	scene->entityCount++;

	return ST_SUCCESS;
}

static StResult setScene(Scene* sc)
{
	for (uint32_t i = 0; i < sc->entityCount; ++i)
	{
		StRenderable renderable = {
			.meshID = sc->entities[i].mesh.meshID,
			.isVisible = sc->entities[i].mesh.isVisible,
		};

		glm_mat4_identity(renderable.modelMatrix);

		// apply transform
		vec3 x = { 1.0f, 0.0f, 0.0f };
		vec3 y = { 0.0f, 1.0f, 0.0f };
		vec3 z = { 0.0f, 0.0f, 1.0f };
		
		glm_translate(renderable.modelMatrix, sc->entities[i].transform.position);
		glm_rotate(renderable.modelMatrix, sc->entities[i].transform.rotation[0], x);
		glm_rotate(renderable.modelMatrix, sc->entities[i].transform.rotation[1], y);
		glm_rotate(renderable.modelMatrix, sc->entities[i].transform.rotation[2], z);
		glm_scale(renderable.modelMatrix, sc->entities[i].transform.scale);

		// Add to render queue
		stSubmit(&renderable); 
	}

	return ST_SUCCESS;
}
