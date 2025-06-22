# Setup sample
add_executable(sample-setup
	"setup/main.c"
)

target_link_libraries(sample-setup
	PRIVATE Structa
)

# Renderable sample
add_executable(sample-renderable
	"renderable/main.c"
)

target_link_libraries(sample-renderable
	PRIVATE Structa
)

# ECS sample
add_executable(sample-ecs
	"ecs/main.c"
)

target_link_libraries(sample-ecs
	PRIVATE Structa
)