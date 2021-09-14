cmake_minimum_required(VERSION 3.19)

project(visualiser)

set(CMAKE_BUILD_TYPE Release)

file(GLOB_RECURSE SOURCE_FILES
	${CMAKE_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE HEADER_FILES
	${CMAKE_SOURCE_DIR}/src/*.h)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-std=c++17") 

# OpenGL
find_package(OpenGL REQUIRED)

# SDL2
find_package(SDL2 REQUIRED)

# BASS
if (UNIX)
    include_directories(${CMAKE_SOURCE_DIR}/deps/bass_linux/)
    set(LIB_BASS ${CMAKE_SOURCE_DIR}/deps/bass_linux/libbass.so)
endif (UNIX)

if (WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/deps/bass/)
    set(LIB_BASS ${CMAKE_SOURCE_DIR}/deps/bass/bass.lib)
endif (WIN32)

# GLM
include_directories(${CMAKE_SOURCE_DIR}/deps/)

# GLAD
add_library(GLAD ${CMAKE_SOURCE_DIR}/deps/glad/glad.c)
include_directories($CMAKE_SOURCE_DIR/deps/glad/)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL SDL2::SDL2 GLAD dl ${LIB_BASS} stdc++fs)
