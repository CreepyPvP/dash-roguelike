cmake_minimum_required(VERSION 3.25)

project(engine)

set(ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")

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
add_executable(platform 
    code/platform.h 
    code/win32_platform.cpp 
    code/defines.h 
    code/memory.h 
    code/memory.cpp 
    code/renderer_backend.h 
    code/opengl_renderer.cpp
    code/game_math.h
    code/game_math.cpp
)

target_include_directories(platform 
    PUBLIC external
    PUBLIC code
)

target_link_libraries(platform glfw opengl32 glad user32)

target_compile_definitions(platform PUBLIC PLATFORM)

# game
add_library(game SHARED 
    code/defines.h 
    code/game.h 
    code/game_math.h 
    code/platform.h 
    code/memory.h 
    code/stb_image.h 
    code/game.cpp 
    code/game_math.cpp 
    code/memory.cpp 
)

target_include_directories(game 
    PUBLIC external
    PUBLIC code
)
