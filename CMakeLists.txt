cmake_minimum_required(VERSION 3.20)
project(implementation_is_illustration VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(III_ENABLE_VISUALS "Enable visualization capabilities" ON)

# Dependencies
# We expect Eigen3 to be available (e.g., via vcpkg or system install)
# For this environment, we might need to be flexible.
# Since we cloned autodiff, we can use its include path if needed, or fetch generic eigen.
# Ideally autodiff should be proper dependency.


# Manual dependencies (FetchContent bypass)
set(eigen3_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/eigen")

# Library
add_library(iii INTERFACE)
target_include_directories(iii INTERFACE 
    include
    ${eigen3_SOURCE_DIR}
)

if(III_ENABLE_VISUALS)
    target_compile_definitions(iii INTERFACE III_ENABLE_VISUALS)
    
    # We need a source file for the Recorder if visuals are enabled
    # BUT, to keep it simple header-only for now, let's keep Recorder inline or static.
    # If we need a .cpp, we change to STATIC/SHARED library.
    # Let's make it a library that CAN have sources.
endif()

# We might strictly need a source file to generic a proper target if not INTERFACE.
# For now, let's stick to header-only wrapper + a source for the Recorder implementation.
add_library(iii_core STATIC src/Recorder.cpp)
target_link_libraries(iii_core PUBLIC iii)
target_include_directories(iii_core PUBLIC include)

# Example
add_executable(poc_example examples/basic_usage.cpp)
target_link_libraries(poc_example PRIVATE iii_core)

# Optional: Include OpenGL PoC if found as sibling or child
# Optional: Include OpenGL PoC if found as sibling or child
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/iii_opengl_poc")
    message(STATUS "Found iii_opengl_poc, adding to build...")
    # This brings in the 'glfw' target via FetchContent inside iii_opengl_poc/CMakeLists.txt
    add_subdirectory("iii_opengl_poc" build_opengl_poc)
    
    # ---------------------------------------------------------
    # Visualizer App (ImGui enabled)
    # We define it here to borrow sources and dependencies
    # ---------------------------------------------------------
    
    find_package(OpenGL REQUIRED)

    # ImGui
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui")
    file(GLOB IMGUI_SOURCES
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui_demo.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    )

    add_executable(iii_visualizer 
        iii_opengl_poc/src/main.cpp 
        iii_opengl_poc/src/GLRenderer.cpp
        iii_opengl_poc/src/VisualizerApp.cpp
        ${IMGUI_SOURCES}
        libs/glad/src/glad.c
    )

    target_include_directories(iii_visualizer PRIVATE 
        iii_opengl_poc/src 
        ${IMGUI_DIR} 
        ${IMGUI_DIR}/backends
        libs/glad/include
    )
    # Link against 'glfw' target provided by iii_opengl_poc subdirectory
    target_link_libraries(iii_visualizer PRIVATE iii_core iii_demos glfw OpenGL::GL)
    target_compile_definitions(iii_visualizer PRIVATE III_ENABLE_VISUALS)
    
endif()

# Benchmark
add_executable(iii_benchmark examples/benchmark_mixed.cpp)
target_link_libraries(iii_benchmark PRIVATE iii_core)

# Demos Library (Shared Examples)
add_library(iii_demos INTERFACE)
target_include_directories(iii_demos INTERFACE examples/shared)
target_link_libraries(iii_demos INTERFACE iii_core)

# Dot/Cross Verification
add_executable(example_dot_cross examples/example_dot_cross.cpp)
target_link_libraries(example_dot_cross PRIVATE iii_core)

# Matrix Verification
add_executable(example_matrix examples/example_matrix.cpp)
target_link_libraries(example_matrix PRIVATE iii_core iii_demos)

# Parameter Benchmark
add_executable(benchmark_parameter examples/benchmark_parameter.cpp)
target_link_libraries(benchmark_parameter PRIVATE iii_core)
