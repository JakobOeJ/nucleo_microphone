set(target_name example_xbee)

set(SRC_FILES
	main.c
)

add_executable(${target_name} ${SRC_FILES})
target_link_libraries(${target_name}
	board_driver
	shield_drivers
)
