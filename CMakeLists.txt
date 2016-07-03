cmake_minimum_required(VERSION 3.0)
project(curvedraw)

file(GLOB hdr "src/*.h")
file(GLOB src "src/*.cc")
file(GLOB main_src "src/main_*.cc")
list(REMOVE_ITEM src ${main_src})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenGL REQUIRED)
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(GLUT)

if(Qt5Widgets_FOUND)
	set(build_qtgui_default ON)
	set(build_glut_default OFF)
else()
	set(build_qtgui_default OFF)
	if(GLUT_FOUND)
		set(build_glut_default ON)
	else()
		set(build_glut_default OFF)
	endif()
endif()

option(build_qtgui "Build with Qt GUI" ${build_qtgui_default})
option(build_glut "Build with simple UI" ${build_glut_default})

if(build_qtgui AND build_glut)
	message(FATAL_ERROR "Can't build both Qt GUI AND simple UI. Pick one")
endif()
if(build_glut AND NOT GLUT_FOUND)
	message(FATAL_ERROR "simple UI needs GLUT (which wasn't found)")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4996 /wd4305")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")
endif()


if(build_qtgui)
	list(APPEND src "${CMAKE_CURRENT_SOURCE_DIR}/src/main_qt.cc")
	set(libs Qt5::Widgets Qt5::OpenGL)
	set(CMAKE_AUTOMOC ON)
endif()
if(build_glut)
	list(APPEND src "${CMAKE_CURRENT_SOURCE_DIR}/src/main_glut.cc")
	set(libs ${GLUT_LIBRARIES})
endif()

add_executable(curvedraw ${src} ${hdr})
set_target_properties(curvedraw PROPERTIES CXX_STANDARD 11)
target_link_libraries(curvedraw ${libs} -lvmath -ldrawtext ${OPENGL_LIBRARIES})

install(TARGETS curvedraw RUNTIME DESTINATION bin)
