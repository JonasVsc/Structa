# Setup sample
add_executable(sample-setup
	"setup/main.c"
)

target_link_libraries(sample-setup
	PRIVATE Structa
)