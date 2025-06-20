#ifndef STRUCTA_COMPONENTS_H
#define STRUCTA_COMPONENTS_H

#include <stdint.h>
#include <stdbool.h>
#include <cglm/cglm.h>


// =============================================================================
// Entity Definition
// =============================================================================

typedef uint32_t StEntity; // Uma entidade é apenas um ID.

// =============================================================================
// Component Definitions
// =============================================================================

typedef struct MeshComponent {
	uint32_t meshID;
	bool isVisible;
} MeshComponent;

typedef struct TransformComponent {
	vec3 position;
	vec3 rotation;
	vec3 scale;
} TransformComponent;

// ... outros componentes (física, câmera, etc.)

// =============================================================================
// Scene Definition
// =============================================================================

#define MAX_ENTITIES 5000



#endif // STRUCTA_COMPONENTS_H