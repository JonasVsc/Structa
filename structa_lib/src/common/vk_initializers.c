#include "vk_initializers.h"
#include "utils.h"

VkShaderModule vk_create_shader_module(VkDevice device, const char* path)
{
	size_t code_size;
	DWORD* p_code = read_file(path, &code_size);
	if (p_code == NULL) return NULL;

	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code_size,
		.pCode = (uint32_t*)p_code
	};

	VkShaderModule shader_module = { 0 };
	vkCreateShaderModule(device, &shader_module_create_info, NULL, &shader_module);
	return shader_module;
}
