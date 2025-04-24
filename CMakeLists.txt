cmake_minimum_required (VERSION 3.16)

project (viewer)

find_package(OpenGL REQUIRED)

#Tells compiler to use c++ 20
set(CMAKE_CXX_STANDARD 20)
#Makes c++ 20 required
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/")
set(IMGUI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/imgui")
set(GLFW_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/glfw")
set(GLEW_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/glew")
set(GLM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/glm")
set(STB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/stb")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/External/raudio)

#GLFW settings
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

file (GLOB SOURCE_FILES "${SOURCE_DIR}/*.cpp")
add_subdirectory(${GLFW_SOURCE_DIR})
add_subdirectory(${GLM_SOURCE_DIR})
add_subdirectory(${IMGUI_SOURCE_DIR})
add_library(StaticGLEW STATIC ${GLEW_SOURCE_DIR}/src/glew.c)

include_directories(
        ${GLFW_SOURCE_DIR}/include
        ${GLEW_SOURCE_DIR}/include
        ${IMGUI_SOURCE_DIR}
        ${STB_SOURCE_DIR}
)
add_executable (${PROJECT_NAME} ${SOURCE_FILES}
        src/window.cpp
        src/window.h
        src/camera.cpp
        src/camera.h
        src/audio.cpp
        src/audio.h
        src/Quad.cpp
        src/terrain.cpp
        src/terrain.h
        src/texture.cpp)

#target_link_libraries(${PROJECT_NAME} PRIVATE StaticGLEW)
#target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE glm)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE raudio)

if (WIN32)
    add_compile_definitions(GLEW_STATIC)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
    target_link_libraries(${PROJECT_NAME} PRIVATE glu32)
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE GL)
endif()