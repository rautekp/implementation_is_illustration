# Implementation is Illustration (iii)

**Implementation is Illustration** is a C++ library designed to bridge the gap between high-performance mathematical code and visual explanation. It allows you to write standard mathematical C++ code that can double as its own visualization, with zero runtime overhead when visualization is disabled.

## Purpose

Complex mathematical algorithms are often hard to debug and even harder to explain. Traditional visualization requires writing separate code, often leading to divergence between the "real" implementation and the "visual" one.

**iii** solves this by augmenting standard types (like 3D vectors) with optional hook capabilities.
- **Visual Mode**: Operations record their history, context, and relationships (A + B -> C).
- **Fast Mode**: The hooks compile away completely, leaving you with raw Eigen-powered performance.

## Features

-   **Zero-Overhead "Fast" Mode**: Compiles down to optimized inline code (verified to match raw Eigen performance).
-   **Drop-in Compatibility**: Types inherit from and interoperate with `Eigen::Vector` types.
-   **Function Wrappers**: `iii::Function` automatically logs function entry/exit and arguments for visual debugging.
-   **Event System**: Flexible `Recorder` and `IEventListener` architecture to stream events to files, consoles, or real-time renderers.

## Project Structure

-   `include/iii/`: Core library headers.
-   `examples/`: Usage examples and benchmarks.
-   `iii_opengl_poc/`: A Proof-of-Concept OpenGL renderer that visualizes the library's output.

## Build Instructions

This project uses CMake and FetchContent. It is self-contained and will download dependencies (Eigen, GLFW, etc.) automatically.

### Prerequisites
-   CMake 3.20+
-   C++20 compliant compiler (MSVC, Clang 10+, GCC 10+)
-   System OpenGL drivers

### Building

```bash
# 1. Configure
cmake -S . -B build

# 2. Build everything
cmake --build build --config Release
```

### Running Examples

**Basic Usage:**
```bash
./build/Release/poc_example.exe
```

**Benchmarks:**
```bash
./build/Release/iii_benchmark.exe
```

### Running the Visualizer with PoC
If you built the project with default options, the OpenGL PoC is included.

```bash
./build/Release/iii_opengl_poc.exe
```

## License
[License Info Here]
