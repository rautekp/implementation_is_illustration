#include <iii/Vector.hpp>
#include <iostream>

using Vector3 = iii::Vector3<double, true>; // Force visual mode

int main() {
  std::cout << "Starting Dot/Cross Product Verification..." << std::endl;

  // 1. Setup
  Vector3 x(1.0, 0.0, 0.0);
  x.set_label("X-Axis");
  x.set_color(1.0f, 0.0f, 0.0f);

  Vector3 y(0.0, 1.0, 0.0);
  y.set_label("Y-Axis");
  y.set_color(0.0f, 1.0f, 0.0f);

  // 2. Cross Product (Should produce Z-axis)
  // Logic: X x Y = Z
  Vector3 z = x.cross(y);
  z.set_label("Z-Axis (Cross)");
  z.set_color(0.0f, 0.0f, 1.0f);
  
  std::cout << "Cross Product (X x Y): " << z << std::endl;

  // 3. Dot Product
  // Logic: X . Y = 0 (Orthogonal)
  // Logic: X . X = 1 (Parallel)
  double dot_ortho = x.dot(y);
  double dot_parallel = x.dot(x);

  std::cout << "Dot Product (Ortho): " << dot_ortho << std::endl;
  std::cout << "Dot Product (Parallel): " << dot_parallel << std::endl;

  // 4. Dump Trace
  iii::Recorder::get().dump("trace_dot_cross.json");

  return 0;
}
