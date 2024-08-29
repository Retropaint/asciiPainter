# cmake setup
cmake_minimum_required(VERSION 3.29.1) 

# compiler setup
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# main executable setup
project(asciiPainter)
add_executable(asciiPainter main.cpp)

# lists, for organization and de-clutter
set(includes)
set(libs)
set(dirLinks)

# global include dirs
list(APPEND includes 
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
)

# platform-specific setup
if(UNIX)
	list(APPEND libs ncurses)
else(WIN32)
	# PDCurses & PDCursesMod setup
	option(OG_PDCURSES "Set to ON to use original PDCurses instead of PDCursesMod" 0)
	if(OG_PDCURSES)
		set(pdcursesPath "${CMAKE_CURRENT_SOURCE_DIR}/deps/pdcurses")
		set(pdcursesLib pdcurses)
	else()
		set(pdcursesPath "${CMAKE_CURRENT_SOURCE_DIR}/deps/pdcurses/pdcursesMod")
		set(pdcursesLib pdcursesMod)
	endif()
	list(APPEND includes	${pdcursesPath})
	list(APPEND dirLinks	${pdcursesPath})
	list(APPEND libs		${pdcursesLib})
endif()

# local libs setup
add_subdirectory(agnos)
list(APPEND libs agnos)

# put those lists to use!
target_include_directories(	asciiPainter PRIVATE ${includes})
target_include_directories(	agnos PRIVATE ${includes})
target_link_libraries(		asciiPainter PRIVATE ${libs})
target_link_directories(	asciiPainter PRIVATE ${dirLinks})
