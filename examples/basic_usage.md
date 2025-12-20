# Basic Usage Example

This example demonstrates the core features of the **Implementation is Illustration (iii)** library.

**File:** `basic_usage.cpp`

## What it does
1.  **Implements a Custom Listener**: Shows how to inherit from `iii::IEventListener` to process real-time events.
2.  **Creates Visual Objects**: Instantiates `iii::Vector3` objects which are automatically tracked.
3.  **Sets Visual Properties**: Sets labels and colors for better visualization.
4.  **Uses Mathematical Functions**: Calls `iii::lerp`, triggering function entry/exit events.
5.  **Dumps Trace**: Saves the session history to `trace.json`.

## How to Run
```sh
# Build the project
cmake --build build --target poc_example

# Run
./build/Debug/poc_example.exe 
# or ./build/Release/poc_example.exe
```
