cmake_minimum_required (VERSION 3.31)
project ("Structa")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(VULKAN_SDK "C:\\VulkanSDK\\1.4.313.0")
include_directories(src ${VULKAN_SDK}/Include)

set(WINDOW_SRC
	"src/window.c"
)

set(RENDERER_SRC
	"src/renderer.c"
)

add_executable (${PROJECT_NAME}
	"src/main.c"
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
	PRIVATE ${VULKAN_LIB} ${SDL2_LIB}
)