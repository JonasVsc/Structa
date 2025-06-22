# Setup sample
add_executable(sample-setup
	"setup/main.c"
)

target_link_libraries(sample-setup
	PRIVATE Structa
)

# ECS sample
add_executable(sample-ecs
	"ecs/main.c"
)

target_link_libraries(sample-ecs
	PRIVATE Structa
)