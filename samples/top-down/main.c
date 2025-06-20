#include "structa.h"
#include <stdio.h>

#include "utils/helper.h"

typedef struct Application {
	StWindow window;
} Application;

static Application app = { 0 };

int main(int argc, char** argv)
{
	StWindowCreateInfo windowCI = {
		.title = "teste",
		.width = 640,
		.height = 480
	};
	ST_CHECK(stCreateWindow(&windowCI, &app.window));
	ST_CHECK(stCreateRenderer(&app.window));


	while (!app.window.shouldClose)
	{
		stPoolEvents(&app.window);
		stRender();
	}

	stDestroyRenderer();
	stDestroyWindow(&app.window);
	return 0;
}