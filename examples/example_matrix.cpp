#include <iii/Matrix.hpp>
#include <iii/Vector.hpp>
#include <iostream>

using Vector3 = iii::Vector3<double, true>;
using Matrix4 = iii::Matrix4<double, true>;

int main() {
  std::cout << "Starting Matrix Verification..." << std::endl;

  // 1. Create a point
  Vector3 p(1.0, 1.0, 0.0);
  p.set_label("Original Point");
  p.set_color(1.0, 1.0, 0.0);

  // 2. Create Translation Matrix
  Matrix4 T = Matrix4::Translate(2.0, 0.0, 0.0);
  // T.set_label("Translation"); // Not yet implemented

  // 3. Transform
  Vector3 p_prime = T * p;
  p_prime.set_label("Transformed Point");
  p_prime.set_color(0.0, 1.0, 0.0);

  std::cout << "Original: " << p << std::endl;
  std::cout << "Transformed: " << p_prime << std::endl; // Should be [3, 1, 0]

  iii::Recorder::get().dump("trace_matrix.json");
  return 0;
}
