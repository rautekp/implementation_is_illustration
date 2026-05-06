# Implementation is Illustration (iii)

**Implementation is Illustration** (`iii`) is a C++ library designed to bridge the gap between high-performance mathematical code and intuitive visual explanation. It allows you to write standard mathematical C++ code that can double as its own interactive 3D visualization, with **zero runtime overhead** when visualization is disabled.

Never write "real" code and "visualization" code separately again.

## 🌟 The Core Philosophy

Complex mathematical algorithms are often hard to debug and even harder to explain. Traditional visualization requires writing bespoke rendering code that mirrors the algorithmic logic, inevitably leading to divergence between the "real" implementation and the "visual" one.

**iii** solves this by augmenting standard types (like 3D vectors and matrices) with optional event-hook capabilities.

- **Visual Mode**: Operations record their history, geometric context, and relationships (e.g., $A + B \rightarrow C$).
- **Fast Mode**: The hooks compile away completely, leaving you with raw Eigen-powered performance.

## ✨ Features

- **Zero-Overhead "Fast" Mode**: Compiles down to optimized inline math code (verified via benchmarks to match raw Eigen performance).
- **Drop-in Compatibility**: Types inherit from and interoperate seamlessly with `Eigen::Vector` types.
- **Real-Time Interactive Visualizer**: An included ImGui + OpenGL Proof-of-Concept visualizer allows you to scrub backward and forward through algorithmic timelines exactly like a video player.
- **Concurrent Multi-Experiment Execution**: 
  - Compare algorithms side-by-side! Create multiple "Experiment Tabs" in the visualizer to run algorithms with different parameters simultaneously in the same 3D space.
  - Perfect for comparing the divergence of **Adaptive step DP/RK45** versus standard **RK4** integration!
- **Live Parameter Tuning**: Update parameters (like step sizes or error tolerances) from the visualizer UI, and the C++ trace is instantly regenerated and visualized.
- **Function Wrappers**: Use `iii::Function` to automatically log function scope entry/exit and arguments for visual debugging.

## 📂 Project Structure

- `include/iii/`: Core library headers. Add this to your include path to use the visual types.
- `src/`: Core library implementation files (e.g., the global `Recorder`).
- `examples/`: Usage examples, algorithm demonstrations (like RK4/RK5 and Inverse Kinematics), and benchmarks.
- `iii_opengl_poc/`: A self-contained OpenGL + ImGui application that loads and renders the geometric events emitted by your `iii` math.

## 🚀 Build Instructions

This project uses CMake and `FetchContent`. It is self-contained and will download necessary UI dependencies (Eigen, GLFW, ImGui, GLAD) automatically.

### Prerequisites
- CMake 3.20+
- C++20 compliant compiler (MSVC, Clang 10+, GCC 10+)
- OpenGL 4.1+ system drivers

### Compiling

```bash
# 1. Configure the project
cmake -S . -B build

# 2. Build everything (Core, Examples, and the Visualizer)
cmake --build build --config Release
```

## 🎮 Running the Visualizer

The visualizer is the best way to experience the library. It includes demos for Solar System simulations, Inverse Kinematics, Ray-Sphere Intersections, and Adaptive RK45 numerical integration.

```bash
./build/Release/iii_visualizer.exe
```

### Visualizer Controls
- **Timeline:** Use the playback buttons or slider at the bottom to scrub through mathematical events over time.
- **Parameters (Right Panel):** Tweak real-time parameters (e.g., starting conditions, integration step sizes, tolerances). Click the `+` tab to spawn a new concurrent experiment for comparison!
- **Layers (Left Panel):** Toggle the visibility of individual algorithm stages or vectors (e.g., hide the $K_1$ through $K_7$ stages of the Dormand-Prince calculation).
- **Camera:** Click and drag in the viewport to orbit around the 3D representation of your algorithm.

## 📊 Running Benchmarks

Want proof of the zero-overhead claim? Run the included parameter and mixed benchmark executables:

```bash
./build/Release/iii_benchmark.exe
./build/Release/benchmark_parameter.exe
```

*Note: For the truest zero-overhead performance in production, compile with `III_ENABLE_VISUALS` set to `OFF`.*

## 📜 License

This project is licensed under the MIT License. Feel free to use, modify, and distribute it in both open-source and commercial applications.
