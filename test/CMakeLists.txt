# 3 june 2016

if(WIN32)
	set(_TEST_RESOURCES_RC resources.rc)
endif()

_add_exec(tester
	images.c
	main.c
	menus.c
	page1.c
	page2.c
	page3.c
	page4.c
	page5.c
	page11.c
	page12.c
	page14.c
	page16.c
	spaced.c
	${_TEST_RESOURCES_RC}
)
target_include_directories(tester
	PRIVATE test
)
set_target_properties(tester PROPERTIES
	OUTPUT_NAME test
	WIN32_EXECUTABLE FALSE
)
