#include "gui_module.h"
#include "structa_internal.h"

#define IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE (8)

StructaContext g = NULL;

static void checkVkResult(VkResult err)
{
	if (err == VK_SUCCESS)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

static int ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{

	VkWin32SurfaceCreateInfoKHR createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = (HWND)viewport->PlatformHandleRaw;
	createInfo.hinstance = GetModuleHandle(NULL);
	return (int)vkCreateWin32SurfaceKHR((VkInstance)vk_instance, &createInfo, (VkAllocationCallbacks*)vk_allocator, (VkSurfaceKHR*)out_vk_surface);
}


void StructaGuiLoad(StructaContext ctx)
{
	g = ctx;
	printf("[Loader] Loading gui\n");

	// cimgui Context
	igCreateContext(NULL);
	ImGuiIO* io = igGetIO_Nil(); (void)io;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	igStyleColorsDark(NULL);

	StructaRenderer r = &g->renderer;

	ImGui_ImplVulkan_InitInfo initInfo = {
		.ApiVersion = VK_API_VERSION_1_4,
		.Instance = r->instance,
		.PhysicalDevice = r->physicalDevice,
		.Device = r->device,
		.QueueFamily = r->graphicsQueueFamily,
		.Queue = r->graphicsQueue,
		.MinImageCount = 2,
		.ImageCount = r->swapchainImageCount,
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
		.UseDynamicRendering = true,
		.PipelineRenderingCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &r->swapchainFormat.format
		},
		.CheckVkResultFn = checkVkResult
	};

	ImGuiPlatformIO* platform = igGetPlatformIO_Nil();
	platform->Platform_CreateVkSurface = ImGui_ImplWin32_CreateVkSurface;

	ImGui_ImplWin32_Init(g->window.handle);
	ImGui_ImplVulkan_Init(&initInfo);
}

void StructaGuiUnload()
{
	vkDeviceWaitIdle(g->renderer.device);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	igDestroyContext(NULL);

	printf("[Loader] Unloading gui\n");
}

void StructaGuiBeginFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	igNewFrame();
} 

void StructaGuiDraw()
{
	if (g->gui.visible)
	{
		g->gui.updateFPS_FlagHelper += g->timer.deltaTime;
		if (g->gui.updateFPS_FlagHelper > 0.5f)
		{
			g->gui.frameTime = g->timer.smoothFPS;
			g->gui.updateFPS_FlagHelper = 0.0f;
		}

		ImGuiIO* io = igGetIO_Nil();
		ImVec2 outerSize = { 0.0f, 0.0f };
		ImVec2 windowPos = { io->DisplaySize.x, 0 };
		ImVec2 pivot = { 1.3f, 0.0f };
		igSetNextWindowPos(windowPos, ImGuiCond_Always, pivot);
		
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoInputs;

		igBegin("Awesome Window", NULL, flags);
		igText("%.1f FPS", g->gui.frameTime);
		igEnd();
	}
}

void StructaGuiEndFrame()
{
	igRender();
}

void StructaGuiRenderDrawData()
{
	ImGui_ImplVulkan_RenderDrawData(igGetDrawData(), g->renderer.commandBuffers[g->renderer.frame], NULL); 
}

void StructaGuiUpdatePlatform()
{
	igUpdatePlatformWindows();
	igRenderPlatformWindowsDefault(NULL, NULL);
}

LRESULT StructaWndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KEYDOWN && wParam == VK_F1)
	{
		g->gui.visible = !g->gui.visible;
		return 0;
	} 

	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
}

