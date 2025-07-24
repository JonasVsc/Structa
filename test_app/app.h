#define USE_STRUCTA_GUI
#include "structa.h"

typedef struct StApplication {
	StWindow window;
	StRenderer renderer;
} StApplication;

void setup();

void run();

void cleanup();