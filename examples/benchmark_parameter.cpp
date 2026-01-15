#define III_NO_VISUALS_OVERRIDE // Uncomment to test "Fast Path" (Zero
// Overhead)
#include <chrono>
#include <iii/Parameter.hpp>
#include <iostream>
#include <vector>

// Explicit benchmark for native double
double bench_native(int N) {
  auto start = std::chrono::high_resolution_clock::now();
  double p = 1.0000001; // Use variable to match Parameter behavior
  double result = 1.0;
  for (int i = 0; i < N; ++i) {
    result = result * p + 0.000001;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  if (result == 0.0)
    std::cout << ""; // Prevent optimize
  std::cout << "[Native Double] " << diff.count() << "s, result=" << result
            << "\n";
  return result;
}

// Explicit benchmark for Parameter
double bench_param(int N) {
  auto start = std::chrono::high_resolution_clock::now();
  iii::Parameter<double> val("bench_p", 1.0000001);
  // Copy to implicit type for loop? No, use it directly?
  // Using it directly mimics 'using a parameter in computation'
  // But result assignment usually converts it to double.
  // So:
  double result = 1.0;
  for (int i = 0; i < N; ++i) {
    // Here we access 'val' every iteration
    result = result * (double)val + 0.000001;
    // Note: casting to double or implicit conversion
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  if (result == 0.0)
    std::cout << "";
  std::cout << "[iii::Parameter] " << diff.count() << "s, result=" << result
            << "\n";
  return result;
}

// Explicit benchmark for Parameter Creation (Heavy overhead expected if Visuals
// ON)
void bench_creation(int N) {
  auto start = std::chrono::high_resolution_clock::now();
  double accumulator = 0.0;
  for (int i = 0; i < N; ++i) {
    // Construct parameter inside loop
    iii::Parameter<double> val("loop_param",
                               1.0); // Registry lookup every time!
    accumulator += (double)val;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  if (accumulator == 0.0)
    std::cout << "";
  std::cout << "[iii::Parameter Creation] " << diff.count() << "s ("
            << (N / diff.count() / 1e6) << " Mops/s)\n";
}

// Explicit benchmark for Native Creation (Baseline)
void bench_native_creation(int N) {
  auto start = std::chrono::high_resolution_clock::now();
  double accumulator = 0.0;
  for (int i = 0; i < N; ++i) {
    double val = 1.0;
    accumulator += val;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  if (accumulator == 0.0)
    std::cout << "";
  std::cout << "[Native Double Creation] " << diff.count() << "s\n";
}

int main() {
  int N = 100000000;
  std::cout << "Benchmarking Access N=" << N << "\n";
  double r1 = bench_native(N);
  double r2 = bench_param(N);

  if (std::abs(r1 - r2) > 1e-9) {
    std::cerr << "MISMATCH! native=" << r1 << ", param=" << r2 << "\n";
  } else {
    std::cout << "Results match: " << r1 << "\n";
  }

  int N_create = 100000000; // 100 Million creations
  std::cout << "\nBenchmarking Creation N=" << N_create << "\n";
  bench_native_creation(N_create);
  bench_creation(N_create);

  return 0;
}
