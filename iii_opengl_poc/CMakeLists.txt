cmake_minimum_required(VERSION 3.20)
project(iii_opengl_poc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip
)
FetchContent_MakeAvailable(glfw)

# Find System OpenGL
find_package(OpenGL REQUIRED)

# Include sibling library 'iii'
# Check if target already exists (e.g. if included FROM iii)
if(NOT TARGET iii)
    message(WARNING "iii target not found. This project expects to be built from the root implementation_is_illustration.")
endif()

# Executable
add_executable(iii_opengl_poc
    src/main.cpp
    src/GLRenderer.cpp
)

target_link_libraries(iii_opengl_poc PRIVATE
    iii_core
    glfw
    OpenGL::GL
)

target_include_directories(iii_opengl_poc PRIVATE src)
