#ifndef RENDERER_H
#define RENDERER_H

// =============================================================================
// Dependencies
// =============================================================================
#include "core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <cglm/cglm.h>

// =============================================================================
// Forward Declarations & Opaque Pointers
// =============================================================================
typedef struct StWindow StWindow;

// =============================================================================
// Public Data Structures
// =============================================================================

/**
 * @brief Representa um objeto renderizável na cena.
 */
typedef struct StRenderable {
	mat4 modelMatrix;
	uint32_t meshID;
	bool isVisible;
} StRenderable;

/**
 * @brief Informações para criação de um mesh. [TEMPORARIO]
 */
typedef struct StMeshCreateInfo {
	const void* src;
	size_t size;
	uint32_t vertexCount;
} StMeshCreateInfo;

// =============================================================================
// Renderer Lifecycle API
// =============================================================================

/**
 * @brief Cria e inicializa a instância do renderer e todos os recursos do Vulkan.
 * @param window Ponteiro para uma janela já inicializada.
 * @return StResult indicando sucesso ou falha.
 */
StResult stCreateRenderer(StWindow* window);

/**
 * @brief Destrói o renderer e libera todos os recursos do Vulkan.
 */
StResult stDestroyRenderer();


// =============================================================================
// Frame & Drawing API
// =============================================================================

/**
 * @brief Executa o loop de renderização para um único frame.
 * @param renderer O renderer que irá desenhar.
 * @return StResult indicando sucesso (ex: frame desenhado) ou falha (ex: swapchain recriado).
 */
void stRender();

/**
 * @brief Submete um objeto para ser desenhado no próximo frame.
 * @param renderer O renderer que irá processar o objeto.
 * @param renderable O objeto a ser renderizado.
 * @return StResult indicando sucesso ou falha.
 */
StResult stSubmit(StRenderable* renderable);


// =============================================================================
// Resource Management API
// =============================================================================

/**
 * @brief Carrega um mesh para a GPU.
 * @param renderer O renderer (necessário para acesso ao device Vulkan).
 * @param createInfo Estrutura com os dados do mesh.
 * @param outMeshID Ponteiro para receber o ID do mesh criado.
 * @return StResult indicando sucesso ou falha.
 */
StResult stCreateMesh(const StMeshCreateInfo* createInfo, uint32_t* id);

#endif // RENDERER_H