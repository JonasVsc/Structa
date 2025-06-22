#ifndef STRUCTA_GAME_BASE_H
#define STRUCTA_GAME_BASE_H

// =============================================================================
// Dependencies
// =============================================================================
#include "renderer.h"

typedef uint32_t EntityID;

// =============================================================================
// Components
// =============================================================================



// =============================================================================
// Public Data Structures
// =============================================================================

typedef struct StEntityCreateInfo {
	TransformCreateInfo transformCreateInfo;
} StEntityCreateInfo;

// =============================================================================
// Lifecycle API
// =============================================================================

void stInitGamebaseSubsystem(StRenderer renderer);

StResult stCreateEntity(const StEntityCreateInfo* createInfo, EntityID* ID);

void stDestroyEntity(EntityID ID);

void stTransformSetPosition(EntityID id, vec3 pos);

void stTransformSetRotation(EntityID id, vec3 rot);

void stTransformSetScale(EntityID id, vec3 sc);
#endif // STRUCTA_GAME_BASE_H