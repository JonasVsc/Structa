cmake_minimum_required (VERSION 3.31)
project ("Structa")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_C_EXTENSIONS OFF)

set(VULKAN_SDK "C:\\VulkanSDK\\1.4.313.2")
include_directories(src ${VULKAN_SDK}/Include)

set(WINDOW_SRC
	"src/window.c"
)

set(RENDERER_SRC
	"src/renderer.c"
)

add_library (${PROJECT_NAME} SHARED
	${RENDERER_SRC}
	${WINDOW_SRC}
)

# 3RDPARTY LIBRARIES

set(SDL2_LIB
	"${VULKAN_SDK}/Lib/SDL2main.lib"
	"${VULKAN_SDK}/Lib/SDL2.lib"
)

set(VULKAN_LIB
	"${VULKAN_SDK}/Lib/vulkan-1.lib"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC ${VULKAN_LIB} ${SDL2_LIB}
)

add_subdirectory(samples)