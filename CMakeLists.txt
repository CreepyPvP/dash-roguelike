cmake_minimum_required(VERSION 3.25)

project(engine)

set(ROOT_DIR ${PROJECT_SOURCE_DIR})

# add_definitions( -DSUPPORT_FILEFORMAT_PNG )

# add_subdirectory(${RAYLIB_DIR})


file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/code/*.cpp
)

file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/code/*.h
)

IF (WIN32)
    find_package(OpenGL REQUIRED)

    set(LIBS glfw opengl32 glad)
ELSE()
    set(LIBS glfw X11 glad)
ENDIF()

add_subdirectory(external/glfw)
add_subdirectory(external/glad)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LIBS})

target_include_directories(${PROJECT_NAME} 
    PUBLIC external
    PUBLIC code
)
