#pragma once

#include "DemoUtils.hpp"
#include <cmath>
#include <iii/Recorder.hpp>
#include <iii/Vector.hpp>

namespace DemoCases {

// ============================================================
// RK4 Integration Demo
//
// Visualizes the 4 stages of Runge-Kutta 4th-order integration
// on a 2D vector field. Each step shows:
//   k1 (red), k2 (orange), k3 (yellow), k4 (green)
//   and the final weighted step (white).
//
// The field is a spiral: f(x,y) = (-y + 0.3x, x + 0.3y)
// so the exact trajectory is an outward spiral.
// ============================================================

using Vec3 = iii::Vector3<double, true>;

inline std::vector<iii::Event> generate_rk4_demo() {
  auto *listener = new VectorListener();
  iii::Recorder::get().addListener(listener);

  // Camera
  iii::EventSetCamera cam;
  cam.id = 0; cam.pitch = 1.4f; cam.yaw = 0.0f;
  cam.dist = 8.0f; cam.fov = 45.0f;
  iii::Recorder::get().record(cam);

  iii::Recorder::get().record(
      iii::EventMessage{0, "RK4 Integration Demo",
        "Visualizing the 4 stages of each Runge-Kutta step"});

  // Vector field: 2D spiral  f(x,y) = (-y + 0.3*x, x + 0.3*y, 0)
  auto field = [](double x, double y) -> std::pair<double, double> {
    return {-y + 0.3 * x, x + 0.3 * y};
  };

  // Initial condition
  double x = 1.0, y = 0.0;
  double h = 0.3; // step size
  int numSteps = 12;

  // Draw the trajectory point at start
  Vec3 pos(x, y, 0.0);
  pos.set_label("x0");
  pos.set_color(1.0f, 1.0f, 1.0f);
  pos.set_class("trajectory_point");
  pos.set_layer("trajectory");

  iii::Recorder::get().record(
      iii::EventMessage{0, "Initial condition: (1, 0)",
        "double x = 1.0, y = 0.0;\ndouble h = 0.3;"});

  for (int step = 0; step < numSteps; ++step) {
    std::string stepStr = "Step " + std::to_string(step + 1);

    // -------- k1 --------
    iii::Recorder::get().record(iii::EventMessage{
        0, stepStr + ": Computing k1 = h * f(x, y)",
        "auto [fx, fy] = f(x, y);\nk1x = h * fx;\nk1y = h * fy;"});

    auto [f1x, f1y] = field(x, y);
    double k1x = h * f1x, k1y = h * f1y;

    Vec3 k1_origin(x, y, 0.0);
    k1_origin.set_visible(false);

    Vec3 k1_vec(k1x, k1y, 0.0);
    k1_vec.set_label("k1");
    k1_vec.set_color(1.0f, 0.2f, 0.2f);
    k1_vec.set_semantic("Vector");
    k1_vec.set_origin(k1_origin);
    k1_vec.set_class("rk_stage");
    k1_vec.set_layer("k1");

    // -------- k2 --------
    iii::Recorder::get().record(iii::EventMessage{
        0, stepStr + ": Computing k2 = h * f(x + k1/2)",
        "k2x = h * f(x + k1x/2, y + k1y/2).x;\nk2y = h * f(x + k1x/2, y + k1y/2).y;"});

    double mx2 = x + k1x * 0.5, my2 = y + k1y * 0.5;
    auto [f2x, f2y] = field(mx2, my2);
    double k2x = h * f2x, k2y = h * f2y;

    // Show midpoint where k2 is evaluated
    Vec3 k2_pt(mx2, my2, 0.0);
    k2_pt.set_visible(false);

    Vec3 k2_vec(k2x, k2y, 0.0);
    k2_vec.set_label("k2");
    k2_vec.set_color(1.0f, 0.6f, 0.1f);
    k2_vec.set_semantic("Vector");
    k2_vec.set_origin(k2_pt);
    k2_vec.set_class("rk_stage");
    k2_vec.set_layer("k2");

    // -------- k3 --------
    iii::Recorder::get().record(iii::EventMessage{
        0, stepStr + ": Computing k3 = h * f(x + k2/2)",
        "k3x = h * f(x + k2x/2, y + k2y/2).x;\nk3y = h * f(x + k2x/2, y + k2y/2).y;"});

    double mx3 = x + k2x * 0.5, my3 = y + k2y * 0.5;
    auto [f3x, f3y] = field(mx3, my3);
    double k3x = h * f3x, k3y = h * f3y;

    Vec3 k3_pt(mx3, my3, 0.0);
    k3_pt.set_visible(false);

    Vec3 k3_vec(k3x, k3y, 0.0);
    k3_vec.set_label("k3");
    k3_vec.set_color(1.0f, 0.9f, 0.1f);
    k3_vec.set_semantic("Vector");
    k3_vec.set_origin(k3_pt);
    k3_vec.set_class("rk_stage");
    k3_vec.set_layer("k3");

    // -------- k4 --------
    iii::Recorder::get().record(iii::EventMessage{
        0, stepStr + ": Computing k4 = h * f(x + k3)",
        "k4x = h * f(x + k3x, y + k3y).x;\nk4y = h * f(x + k3x, y + k3y).y;"});

    double mx4 = x + k3x, my4 = y + k3y;
    auto [f4x, f4y] = field(mx4, my4);
    double k4x = h * f4x, k4y = h * f4y;

    Vec3 k4_pt(mx4, my4, 0.0);
    k4_pt.set_visible(false);

    Vec3 k4_vec(k4x, k4y, 0.0);
    k4_vec.set_label("k4");
    k4_vec.set_color(0.2f, 0.9f, 0.2f);
    k4_vec.set_semantic("Vector");
    k4_vec.set_origin(k4_pt);
    k4_vec.set_class("rk_stage");
    k4_vec.set_layer("k4");

    // -------- Weighted sum --------
    iii::Recorder::get().record(iii::EventMessage{
        0, stepStr + ": Weighted sum: dx = (k1 + 2*k2 + 2*k3 + k4) / 6",
        "x += (k1x + 2*k2x + 2*k3x + k4x) / 6.0;\ny += (k1y + 2*k2y + 2*k3y + k4y) / 6.0;"});

    double dx = (k1x + 2 * k2x + 2 * k3x + k4x) / 6.0;
    double dy = (k1y + 2 * k2y + 2 * k3y + k4y) / 6.0;

    // Show the final step vector
    Vec3 step_origin(x, y, 0.0);
    step_origin.set_visible(false);

    Vec3 step_vec(dx, dy, 0.0);
    step_vec.set_label("dx");
    step_vec.set_color(0.9f, 0.9f, 1.0f);
    step_vec.set_semantic("Vector");
    step_vec.set_origin(step_origin);
    step_vec.set_class("rk_result");
    step_vec.set_layer("result");

    // Advance
    x += dx;
    y += dy;

    // Mark new position
    Vec3 newPos(x, y, 0.0);
    newPos.set_label("x" + std::to_string(step + 1));
    newPos.set_color(1.0f, 1.0f, 1.0f);
    newPos.set_class("trajectory_point");
    newPos.set_layer("trajectory");
  }

  iii::Recorder::get().record(iii::EventMessage{
      0, "RK4 integration complete — 12 steps on spiral field",
      "// Toggle k1/k2/k3/k4 layers to see individual stages\n"
      "// Set 'rk_stage' class to Arrow for better visualization"});

  iii::Recorder::get().removeListener(listener);
  return listener->events;
}

} // namespace DemoCases
