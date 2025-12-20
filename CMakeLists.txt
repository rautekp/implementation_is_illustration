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


# Fetch Eigen if not present
include(FetchContent)
FetchContent_Declare(
  Eigen3
  URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
)
FetchContent_GetProperties(Eigen3)
if(NOT eigen3_POPULATED)
  FetchContent_Populate(Eigen3)
endif()

# Add autodiff
include(FetchContent)
FetchContent_Declare(
  autodiff
  URL https://github.com/autodiff/autodiff/archive/refs/tags/v0.6.13.zip
)
FetchContent_GetProperties(autodiff)
if(NOT autodiff_POPULATED)
  FetchContent_Populate(autodiff)
endif()

# Autodiff is header only, just expose the include dir
# Autodiff is header only, just expose the include dir
add_library(autodiff::autodiff INTERFACE IMPORTED)
target_include_directories(autodiff::autodiff INTERFACE ${autodiff_SOURCE_DIR})
target_include_directories(autodiff::autodiff INTERFACE ${eigen3_SOURCE_DIR})

# Library
add_library(iii INTERFACE)
target_include_directories(iii INTERFACE 
    include
)

target_link_libraries(iii INTERFACE autodiff::autodiff)

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
add_executable(poc_example examples/main.cpp)
target_link_libraries(poc_example PRIVATE iii_core)

# Optional: Include OpenGL PoC if found as sibling or child
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/iii_opengl_poc")
    message(STATUS "Found iii_opengl_poc, adding to build...")
    add_subdirectory("iii_opengl_poc" build_opengl_poc)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../iii_opengl_poc")
     message(STATUS "Found sibling iii_opengl_poc, adding to build...")
     add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../iii_opengl_poc" build_opengl_poc)
endif()

# Benchmark
add_executable(iii_benchmark examples/benchmark_mixed.cpp)
target_link_libraries(iii_benchmark PRIVATE iii_core)
