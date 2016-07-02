cmake_minimum_required(VERSION 3.0)
project(curvedraw)

file(GLOB hdr "src/*.h")
file(GLOB src "src/*.cc")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/src/main_glut.cc")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(OpenGL REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4996 /wd4305")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")
endif()

add_executable(curvedraw ${src} ${hdr})
target_link_libraries(curvedraw Qt5::Widgets Qt5::OpenGL -lvmath -ldrawtext ${OPENGL_LIBRARIES})

set_target_properties(curvedraw PROPERTIES CXX_STANDARD 11)

install(TARGETS curvedraw RUNTIME DESTINATION bin)
