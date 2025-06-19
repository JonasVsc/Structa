#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdint.h>
#include <stdbool.h>

#include <cglm/cglm.h>

typedef uint32_t MeshID;

typedef struct MeshComponent {
	MeshID meshID;
	bool isVisible;
} MeshComponent;

typedef struct TransformComponent {
	vec3 position;
	vec3 rotation;
	vec3 scale;
} TransformComponent;

#endif // COMPONENTS_H