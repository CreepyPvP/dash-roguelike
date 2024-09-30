cmake_minimum_required(VERSION 3.25)

project(engine)

set(ROOT_DIR ${PROJECT_SOURCE_DIR})

# add_definitions( -DSUPPORT_FILEFORMAT_PNG )
# add_subdirectory(${RAYLIB_DIR})

# file(GLOB_RECURSE SOURCE_FILES
#     ${CMAKE_SOURCE_DIR}/code/*.cpp
# )
#
# file(GLOB_RECURSE HEADER_FILES 
# 	${CMAKE_SOURCE_DIR}/code/*.h
# )

IF (WIN32)
    find_package(OpenGL REQUIRED)
ELSE()
    # set(LIBS glfw X11 glad)
ENDIF()

add_subdirectory(external/glfw)
add_subdirectory(external/glad)

# platform layer
add_executable(platform code/platform.cpp code/platform.h code/defines.h code/memory.h code/memory.cpp)

target_include_directories(platform 
    PUBLIC external
    PUBLIC code
)

target_link_libraries(platform glfw opengl32 glad)

# game
add_library(game SHARED 
    code/defines.h 
    code/game.h 
    code/game_math.h 
    code/platform.h 
    code/memory.h 
    code/renderer.h 
    code/stb_image.h 
    code/game.cpp 
    code/game_math.cpp 
    code/memory.cpp 
    code/renderer.cpp)

target_include_directories(game 
    PUBLIC external
    PUBLIC external/glad/include
    PUBLIC code
)

target_link_libraries(game opengl32)
