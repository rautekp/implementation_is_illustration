#include <chrono>
#include <iii/Math.hpp>
#include <iii/Vector.hpp>
#include <iostream>
#include <random>
#include <vector>

// Force compile logic for different types
template <typename VecType>
double run_benchmark(const std::string &name, int interactions) {
  // Generate random vectors
  std::vector<VecType> vecs;
  vecs.reserve(1000);

  std::mt19937 gen(42);
  std::uniform_real_distribution<> dis(-10.0, 10.0);

  for (int i = 0; i < 10000; ++i) {
    vecs.emplace_back(dis(gen), dis(gen), dis(gen));
  }

  auto start = std::chrono::high_resolution_clock::now();

  VecType result(0, 0, 0);
  for (int i = 0; i < interactions; ++i) {
    // complex operation: a + (b-a)*t
    int idxA = i % 1000;
    int idxB = (i * 7) % 1000;
    double t = 0.5;
    result = result + vecs[idxA] + (vecs[idxB] - vecs[idxA]) * t;
  }

  // Prevent optimization
  double val = result.x() + result.y() + result.z();
  if (val == 123456789.0)
    std::cout << "Unlikely";

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;

  std::cout << "[" << name << "] Time: " << diff.count() << "s ("
            << (interactions / diff.count() / 1e6) << " Mops/s)" << std::endl;

  return diff.count();
}

template <typename VecType>
double run_benchmark_func(const std::string &name, int interactions) {
  std::vector<VecType> vecs;
  vecs.reserve(1000);

  std::mt19937 gen(42);
  std::uniform_real_distribution<> dis(-10.0, 10.0);

  for (int i = 0; i < 10000; ++i) {
    vecs.emplace_back(dis(gen), dis(gen), dis(gen));
  }

  auto start = std::chrono::high_resolution_clock::now();

  VecType result(0, 0, 0);
  for (int i = 0; i < interactions; ++i) {
    int idxA = i % 1000;
    int idxB = (i * 7) % 1000;
    double t = 0.5;
    // Use iii::lerp via function wrapper
    result = result + iii::lerp(vecs[idxA], vecs[idxB], t);
  }

  // Prevent optimization
  double val = result.x() + result.y() + result.z();
  if (val == 123456789.0)
    std::cout << "Unlikely";

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;

  std::cout << "[" << name << "] Time: " << diff.count() << "s ("
            << (interactions / diff.count() / 1e6) << " Mops/s)" << std::endl;

  return diff.count();
}

int main() {
  std::cout << "Starting Mixed Mode Benchmark..." << std::endl;
  // III_ENABLE_VISUALS is globally defined in CMake?
  // If so, Recorder is linked. But FastIII should not use it.

  // Warmup
  run_benchmark<Eigen::Vector3d>("Warmup", 1000000);

  int N = 50000000; // 50 Million ops

  std::cout << "\n--- Running Benchmark (N=" << N << ") ---\n";

  // 1. Plain Eigen
  run_benchmark<Eigen::Vector3d>("Plain Eigen", N);

  // 2. Fast III (No Visuals)
  run_benchmark<iii::Vector3<double, false>>("Fast III", N);

  // 3. Visual III (With Recording) - Reduce N for sanity if it's too slow?
  // Recording 5M events might overflow memory or be very slow.
  // Let's use smaller N for visual? Or just accept it takes time.
  // Events are stored in memory in Recorder... 5 million events * sizeof(Event)
  // ~ 5M * 40 bytes = 200MB. Fine.
  run_benchmark<iii::Vector3<double, true>>("Visual III", N / 1000);

  std::cout << "\n--- Function Wrapper Benchmark (N=" << N << ") ---\n";
  run_benchmark_func<iii::Vector3<double, false>>("Fast III (lerp wrapper)", N);
  run_benchmark_func<iii::Vector3<double, true>>("Visual III (lerp wrapper)",
                                                 N / 1000);

  return 0;
}
