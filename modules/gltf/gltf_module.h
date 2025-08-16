#ifndef STRUCTA_GLTF_H_
#define STRUCTA_GLTF_H_ 1

#include "cgltf.h"

typedef struct StructaContext_T StructaContext_T;
typedef struct StructaContext_T* StructaContext;


void StructaGLTFLoad(StructaContext ctx);
void StructaGLTFUnload();

void StructaLoadGLTF(const char* file);

#endif // STRUCTA_GLTF_H_