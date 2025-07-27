#ifndef STRUCTA_EXPERIMENTAL_H_
#define STRUCTA_EXPERIMENTAL_H_ 1

#include <vulkan/vulkan.h>

typedef uint32_t MeshId;

// void stCreateTriangle();

MeshId stLoadMesh();

void stFreeMeshes();

void stDraw(MeshId mesh);

#endif // STRUCTA_EXPERIMENTAL_H_