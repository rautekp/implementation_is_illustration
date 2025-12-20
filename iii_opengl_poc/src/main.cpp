#include "GLRenderer.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

#include <cmath>
#include <iii/Math.hpp>
#include <iii/Vector.hpp>

// Define a type for convenience
// We explicitly enable visualization here because we are in the PoC
using Vector3 = iii::Vector3<double, true>;

int main() {
  std::cout << "Starting OpenGL PoC..." << std::endl;

  GLRenderer renderer;
#ifdef III_ENABLE_VISUALS
  iii::Recorder::get().addListener(&renderer);
#endif

  if (!glfwInit())
    return -1;

  GLFWwindow *window =
      glfwCreateWindow(640, 480, "Implementation is Illustration", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  // Setup vectors
  // Vector Field Interpolation Example

  // 1. Point A and Vector A
  Vector3 pA(-0.5, -0.5, 0.0);
  pA.set_label("P_A");
  pA.set_color(1.0, 0.0, 0.0);

  Vector3 vA(0.0, 0.5, 0.0); // Up vector
  vA.set_semantic("Vector");
  vA.set_origin(pA); // Attach to P_A
  vA.set_color(1.0, 0.5, 0.5);

  // 2. Point B and Vector B
  Vector3 pB(0.5, 0.5, 0.0);
  pB.set_label("P_B");
  pB.set_color(0.0, 0.0, 1.0);

  Vector3 vB(0.5, 0.0, 0.0); // Right vector
  vB.set_semantic("Vector");
  vB.set_origin(pB); // Attach to P_B
  vB.set_color(0.5, 0.5, 1.0);

  std::cout << "Starting loop..." << std::endl;

  while (!glfwWindowShouldClose(window)) {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    // 3. Interpolate
    double time = glfwGetTime();
    double t = (std::sin(time) + 1.0) / 2.0;

    // Interpolate Position
    Vector3 pC = iii::lerp(pA, pB, t);
    pC.set_label("P_C");
    pC.set_color(0.0, 1.0, 0.0);

    // Interpolate Vector
    Vector3 vC = iii::lerp(vA, vB, t);
    vC.set_semantic("Vector");
    vC.set_origin(pC); // Attach to moving point P_C
    vC.set_color(1.0, 1.0, 0.0);

    // vC is always visible
    vC.set_visible(true);

    renderer.render();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

#ifdef III_ENABLE_VISUALS
  iii::Recorder::get().dump("trace.json");
#endif

  glfwTerminate();
  return 0;
}
