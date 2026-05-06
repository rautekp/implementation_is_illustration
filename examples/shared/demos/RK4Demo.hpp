#pragma once

#include "DemoUtils.hpp"
#include <cmath>
#include <iii/Parameter.hpp>
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
  iii::Parameter<double> p_x("RK4 Start X", 1.0);
  iii::Parameter<double> p_y("RK4 Start Y", 0.0);
  iii::Parameter<double> h("RK4 Step Size", 0.3);
  iii::Parameter<int> numSteps("RK4 Num Steps", 12);
  iii::Parameter<int> method("Integration Method (0=RK4, 1=RK5, 2=DP/RK45)", 0);
  iii::Parameter<double> tolerance("RK45 Tolerance", 0.01);

  double x = p_x;
  double y = p_y;

  std::vector<Vec3> trajectory;
  std::vector<Vec3> trajectory_lines;

  // Draw the trajectory point at start
  trajectory.emplace_back(x, y, 0.0);
  auto& pos = trajectory.back();
  pos.set_label("x0");
  pos.set_color(1.0f, 1.0f, 1.0f);
  pos.set_class("trajectory_point");
  pos.set_layer("trajectory");

  iii::Recorder::get().record(
      iii::EventMessage{0, "Initial condition: (1, 0)",
        "double x = 1.0, y = 0.0;\ndouble h = 0.3;"});

  double current_h = h;
  for (int step = 0; step < numSteps; ++step) {
    double dx = 0.0, dy = 0.0;
    bool step_accepted = false;
    do {
      step_accepted = true;
      std::string stepStr = "Step " + std::to_string(step + 1);

      if (method == 0) { // RK4
      // -------- k1 --------
      iii::Recorder::get().record(iii::EventMessage{
          0, stepStr + ": Computing k1 = current_h * f(x, y)",
          "auto [fx, fy] = f(x, y);\nk1x = current_h * fx;\nk1y = current_h * fy;"});

      auto [f1x, f1y] = field(x, y);
      double k1x = current_h * f1x, k1y = current_h * f1y;

      Vec3 k1_origin(x, y, 0.0);
      k1_origin.set_class("stage_point");
      k1_origin.set_layer("k1");
      k1_origin.set_color(1.0f, 0.2f, 0.2f);

      Vec3 k1_vec(k1x, k1y, 0.0);
      k1_vec.set_label("k1");
      k1_vec.set_color(1.0f, 0.2f, 0.2f);
      k1_vec.set_semantic("Vector");
      k1_vec.set_origin(k1_origin);
      k1_vec.set_class("rk_stage");
      k1_vec.set_layer("k1");

      // -------- k2 --------
      iii::Recorder::get().record(iii::EventMessage{
          0, stepStr + ": Computing k2 = current_h * f(x + k1/2)",
          "k2x = current_h * f(x + k1x/2, y + k1y/2).x;\nk2y = current_h * f(x + k1x/2, y + k1y/2).y;"});

      double mx2 = x + k1x * 0.5, my2 = y + k1y * 0.5;
      auto [f2x, f2y] = field(mx2, my2);
      double k2x = current_h * f2x, k2y = current_h * f2y;

      Vec3 k2_pt(mx2, my2, 0.0);
      k2_pt.set_class("stage_point");
      k2_pt.set_layer("k2");
      k2_pt.set_color(1.0f, 0.6f, 0.1f);

      Vec3 k2_vec(k2x, k2y, 0.0);
      k2_vec.set_label("k2");
      k2_vec.set_color(1.0f, 0.6f, 0.1f);
      k2_vec.set_semantic("Vector");
      k2_vec.set_origin(k2_pt);
      k2_vec.set_class("rk_stage");
      k2_vec.set_layer("k2");

      // -------- k3 --------
      iii::Recorder::get().record(iii::EventMessage{
          0, stepStr + ": Computing k3 = current_h * f(x + k2/2)",
          "k3x = current_h * f(x + k2x/2, y + k2y/2).x;\nk3y = current_h * f(x + k2x/2, y + k2y/2).y;"});

      double mx3 = x + k2x * 0.5, my3 = y + k2y * 0.5;
      auto [f3x, f3y] = field(mx3, my3);
      double k3x = current_h * f3x, k3y = current_h * f3y;

      Vec3 k3_pt(mx3, my3, 0.0);
      k3_pt.set_class("stage_point");
      k3_pt.set_layer("k3");
      k3_pt.set_color(1.0f, 0.9f, 0.1f);

      Vec3 k3_vec(k3x, k3y, 0.0);
      k3_vec.set_label("k3");
      k3_vec.set_color(1.0f, 0.9f, 0.1f);
      k3_vec.set_semantic("Vector");
      k3_vec.set_origin(k3_pt);
      k3_vec.set_class("rk_stage");
      k3_vec.set_layer("k3");

      // -------- k4 --------
      iii::Recorder::get().record(iii::EventMessage{
          0, stepStr + ": Computing k4 = current_h * f(x + k3)",
          "k4x = current_h * f(x + k3x, y + k3y).x;\nk4y = current_h * f(x + k3x, y + k3y).y;"});

      double mx4 = x + k3x, my4 = y + k3y;
      auto [f4x, f4y] = field(mx4, my4);
      double k4x = current_h * f4x, k4y = current_h * f4y;

      Vec3 k4_pt(mx4, my4, 0.0);
      k4_pt.set_class("stage_point");
      k4_pt.set_layer("k4");
      k4_pt.set_color(0.2f, 0.9f, 0.2f);

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
          "dx = (k1x + 2*k2x + 2*k3x + k4x) / 6.0;\ndy = (k1y + 2*k2y + 2*k3y + k4y) / 6.0;"});

      dx = (k1x + 2 * k2x + 2 * k3x + k4x) / 6.0;
      dy = (k1y + 2 * k2y + 2 * k3y + k4y) / 6.0;

    } else if (method == 1) { // RK5
      iii::Recorder::get().record(iii::EventMessage{
          0, stepStr + ": Dormand-Prince RK5 (Fixed Step)",
          "Evaluating 7 stages for DP45..."});

      // k1
      auto [f1x, f1y] = field(x, y);
      double k1x = current_h * f1x, k1y = current_h * f1y;
      Vec3 k1_origin(x, y, 0.0);
      k1_origin.set_class("stage_point");
      k1_origin.set_layer("k1");
      k1_origin.set_color(1.0f, 0.2f, 0.2f);
      Vec3 k1_vec(k1x, k1y, 0.0); k1_vec.set_label("k1"); k1_vec.set_color(1.0f, 0.2f, 0.2f);
      k1_vec.set_semantic("Vector"); k1_vec.set_origin(k1_origin); k1_vec.set_class("rk_stage"); k1_vec.set_layer("k1");

      // k2
      double mx2 = x + k1x * (1.0/5.0), my2 = y + k1y * (1.0/5.0);
      auto [f2x, f2y] = field(mx2, my2);
      double k2x = current_h * f2x, k2y = current_h * f2y;
      Vec3 k2_pt(mx2, my2, 0.0);
      k2_pt.set_class("stage_point");
      k2_pt.set_layer("k2");
      k2_pt.set_color(1.0f, 0.6f, 0.1f);
      Vec3 k2_vec(k2x, k2y, 0.0); k2_vec.set_label("k2"); k2_vec.set_color(1.0f, 0.5f, 0.1f);
      k2_vec.set_semantic("Vector"); k2_vec.set_origin(k2_pt); k2_vec.set_class("rk_stage"); k2_vec.set_layer("k2");

      // k3
      double mx3 = x + k1x * (3.0/40.0) + k2x * (9.0/40.0);
      double my3 = y + k1y * (3.0/40.0) + k2y * (9.0/40.0);
      auto [f3x, f3y] = field(mx3, my3);
      double k3x = current_h * f3x, k3y = current_h * f3y;
      Vec3 k3_pt(mx3, my3, 0.0);
      k3_pt.set_class("stage_point");
      k3_pt.set_layer("k3");
      k3_pt.set_color(1.0f, 0.9f, 0.1f);
      Vec3 k3_vec(k3x, k3y, 0.0); k3_vec.set_label("k3"); k3_vec.set_color(1.0f, 0.8f, 0.1f);
      k3_vec.set_semantic("Vector"); k3_vec.set_origin(k3_pt); k3_vec.set_class("rk_stage"); k3_vec.set_layer("k3");

      // k4
      double mx4 = x + k1x * (44.0/45.0) - k2x * (56.0/15.0) + k3x * (32.0/9.0);
      double my4 = y + k1y * (44.0/45.0) - k2y * (56.0/15.0) + k3y * (32.0/9.0);
      auto [f4x, f4y] = field(mx4, my4);
      double k4x = current_h * f4x, k4y = current_h * f4y;
      Vec3 k4_pt(mx4, my4, 0.0);
      k4_pt.set_class("stage_point");
      k4_pt.set_layer("k4");
      k4_pt.set_color(0.2f, 0.9f, 0.2f);
      Vec3 k4_vec(k4x, k4y, 0.0); k4_vec.set_label("k4"); k4_vec.set_color(0.5f, 0.9f, 0.1f);
      k4_vec.set_semantic("Vector"); k4_vec.set_origin(k4_pt); k4_vec.set_class("rk_stage"); k4_vec.set_layer("k4");

      // k5
      double mx5 = x + k1x * (19372.0/6561.0) - k2x * (25360.0/2187.0) + k3x * (64448.0/6561.0) - k4x * (212.0/729.0);
      double my5 = y + k1y * (19372.0/6561.0) - k2y * (25360.0/2187.0) + k3y * (64448.0/6561.0) - k4y * (212.0/729.0);
      auto [f5x, f5y] = field(mx5, my5);
      double k5x = current_h * f5x, k5y = current_h * f5y;
      Vec3 k5_pt(mx5, my5, 0.0);
      k5_pt.set_class("stage_point");
      k5_pt.set_layer("k5");
      k5_pt.set_color(0.1f, 0.9f, 0.5f);
      Vec3 k5_vec(k5x, k5y, 0.0); k5_vec.set_label("k5"); k5_vec.set_color(0.1f, 0.9f, 0.5f);
      k5_vec.set_semantic("Vector"); k5_vec.set_origin(k5_pt); k5_vec.set_class("rk_stage"); k5_vec.set_layer("k5");

      // k6
      double mx6 = x + k1x * (9017.0/3168.0) - k2x * (355.0/33.0) + k3x * (46732.0/5247.0) + k4x * (49.0/176.0) - k5x * (5103.0/18656.0);
      double my6 = y + k1y * (9017.0/3168.0) - k2y * (355.0/33.0) + k3y * (46732.0/5247.0) + k4y * (49.0/176.0) - k5y * (5103.0/18656.0);
      auto [f6x, f6y] = field(mx6, my6);
      double k6x = current_h * f6x, k6y = current_h * f6y;
      Vec3 k6_pt(mx6, my6, 0.0);
      k6_pt.set_class("stage_point");
      k6_pt.set_layer("k6");
      k6_pt.set_color(0.1f, 0.6f, 1.0f);
      Vec3 k6_vec(k6x, k6y, 0.0); k6_vec.set_label("k6"); k6_vec.set_color(0.1f, 0.6f, 1.0f);
      k6_vec.set_semantic("Vector"); k6_vec.set_origin(k6_pt); k6_vec.set_class("rk_stage"); k6_vec.set_layer("k6");

      // k7
      double mx7 = x + k1x * (35.0/384.0) + k3x * (500.0/1113.0) + k4x * (125.0/192.0) - k5x * (2187.0/6784.0) + k6x * (11.0/84.0);
      double my7 = y + k1y * (35.0/384.0) + k3y * (500.0/1113.0) + k4y * (125.0/192.0) - k5y * (2187.0/6784.0) + k6y * (11.0/84.0);
      auto [f7x, f7y] = field(mx7, my7);
      double k7x = current_h * f7x, k7y = current_h * f7y;
      Vec3 k7_pt(mx7, my7, 0.0);
      k7_pt.set_class("stage_point");
      k7_pt.set_layer("k7");
      k7_pt.set_color(0.6f, 0.1f, 1.0f);
      Vec3 k7_vec(k7x, k7y, 0.0); k7_vec.set_label("k7"); k7_vec.set_color(0.6f, 0.1f, 1.0f);
      k7_vec.set_semantic("Vector"); k7_vec.set_origin(k7_pt); k7_vec.set_class("rk_stage"); k7_vec.set_layer("k7");

      // 5th order step
      dx = k1x * (35.0/384.0) + k3x * (500.0/1113.0) + k4x * (125.0/192.0) - k5x * (2187.0/6784.0) + k6x * (11.0/84.0);
      dy = k1y * (35.0/384.0) + k3y * (500.0/1113.0) + k4y * (125.0/192.0) - k5y * (2187.0/6784.0) + k6y * (11.0/84.0);
    } else if (method == 2) { // DP/RK45
      iii::Recorder::get().record(iii::EventMessage{
          0, stepStr + ": True DP/RK45",
          "Evaluating 7 stages...\nUsing current_h = " + std::to_string(current_h)});

      // k1
      auto [f1x, f1y] = field(x, y);
      double k1x = current_h * f1x, k1y = current_h * f1y;
      Vec3 k1_origin(x, y, 0.0);
      k1_origin.set_class("stage_point"); k1_origin.set_layer("k1"); k1_origin.set_color(1.0f, 0.2f, 0.2f);
      Vec3 k1_vec(k1x, k1y, 0.0); k1_vec.set_label("k1"); k1_vec.set_color(1.0f, 0.2f, 0.2f);
      k1_vec.set_semantic("Vector"); k1_vec.set_origin(k1_origin); k1_vec.set_class("rk_stage"); k1_vec.set_layer("k1");

      // k2
      double mx2 = x + k1x * (1.0/5.0), my2 = y + k1y * (1.0/5.0);
      auto [f2x, f2y] = field(mx2, my2);
      double k2x = current_h * f2x, k2y = current_h * f2y;
      Vec3 k2_pt(mx2, my2, 0.0);
      k2_pt.set_class("stage_point"); k2_pt.set_layer("k2"); k2_pt.set_color(1.0f, 0.5f, 0.1f);
      Vec3 k2_vec(k2x, k2y, 0.0); k2_vec.set_label("k2"); k2_vec.set_color(1.0f, 0.5f, 0.1f);
      k2_vec.set_semantic("Vector"); k2_vec.set_origin(k2_pt); k2_vec.set_class("rk_stage"); k2_vec.set_layer("k2");

      // k3
      double mx3 = x + k1x * (3.0/40.0) + k2x * (9.0/40.0);
      double my3 = y + k1y * (3.0/40.0) + k2y * (9.0/40.0);
      auto [f3x, f3y] = field(mx3, my3);
      double k3x = current_h * f3x, k3y = current_h * f3y;
      Vec3 k3_pt(mx3, my3, 0.0);
      k3_pt.set_class("stage_point"); k3_pt.set_layer("k3"); k3_pt.set_color(1.0f, 0.8f, 0.1f);
      Vec3 k3_vec(k3x, k3y, 0.0); k3_vec.set_label("k3"); k3_vec.set_color(1.0f, 0.8f, 0.1f);
      k3_vec.set_semantic("Vector"); k3_vec.set_origin(k3_pt); k3_vec.set_class("rk_stage"); k3_vec.set_layer("k3");

      // k4
      double mx4 = x + k1x * (44.0/45.0) - k2x * (56.0/15.0) + k3x * (32.0/9.0);
      double my4 = y + k1y * (44.0/45.0) - k2y * (56.0/15.0) + k3y * (32.0/9.0);
      auto [f4x, f4y] = field(mx4, my4);
      double k4x = current_h * f4x, k4y = current_h * f4y;
      Vec3 k4_pt(mx4, my4, 0.0);
      k4_pt.set_class("stage_point"); k4_pt.set_layer("k4"); k4_pt.set_color(0.5f, 0.9f, 0.1f);
      Vec3 k4_vec(k4x, k4y, 0.0); k4_vec.set_label("k4"); k4_vec.set_color(0.5f, 0.9f, 0.1f);
      k4_vec.set_semantic("Vector"); k4_vec.set_origin(k4_pt); k4_vec.set_class("rk_stage"); k4_vec.set_layer("k4");

      // k5
      double mx5 = x + k1x * (19372.0/6561.0) - k2x * (25360.0/2187.0) + k3x * (64448.0/6561.0) - k4x * (212.0/729.0);
      double my5 = y + k1y * (19372.0/6561.0) - k2y * (25360.0/2187.0) + k3y * (64448.0/6561.0) - k4y * (212.0/729.0);
      auto [f5x, f5y] = field(mx5, my5);
      double k5x = current_h * f5x, k5y = current_h * f5y;
      Vec3 k5_pt(mx5, my5, 0.0);
      k5_pt.set_class("stage_point"); k5_pt.set_layer("k5"); k5_pt.set_color(0.1f, 0.9f, 0.5f);
      Vec3 k5_vec(k5x, k5y, 0.0); k5_vec.set_label("k5"); k5_vec.set_color(0.1f, 0.9f, 0.5f);
      k5_vec.set_semantic("Vector"); k5_vec.set_origin(k5_pt); k5_vec.set_class("rk_stage"); k5_vec.set_layer("k5");

      // k6
      double mx6 = x + k1x * (9017.0/3168.0) - k2x * (355.0/33.0) + k3x * (46732.0/5247.0) + k4x * (49.0/176.0) - k5x * (5103.0/18656.0);
      double my6 = y + k1y * (9017.0/3168.0) - k2y * (355.0/33.0) + k3y * (46732.0/5247.0) + k4y * (49.0/176.0) - k5y * (5103.0/18656.0);
      auto [f6x, f6y] = field(mx6, my6);
      double k6x = current_h * f6x, k6y = current_h * f6y;
      Vec3 k6_pt(mx6, my6, 0.0);
      k6_pt.set_class("stage_point"); k6_pt.set_layer("k6"); k6_pt.set_color(0.1f, 0.6f, 1.0f);
      Vec3 k6_vec(k6x, k6y, 0.0); k6_vec.set_label("k6"); k6_vec.set_color(0.1f, 0.6f, 1.0f);
      k6_vec.set_semantic("Vector"); k6_vec.set_origin(k6_pt); k6_vec.set_class("rk_stage"); k6_vec.set_layer("k6");

      // k7
      double mx7 = x + k1x * (35.0/384.0) + k3x * (500.0/1113.0) + k4x * (125.0/192.0) - k5x * (2187.0/6784.0) + k6x * (11.0/84.0);
      double my7 = y + k1y * (35.0/384.0) + k3y * (500.0/1113.0) + k4y * (125.0/192.0) - k5y * (2187.0/6784.0) + k6y * (11.0/84.0);
      auto [f7x, f7y] = field(mx7, my7);
      double k7x = current_h * f7x, k7y = current_h * f7y;
      Vec3 k7_pt(mx7, my7, 0.0);
      k7_pt.set_class("stage_point"); k7_pt.set_layer("k7"); k7_pt.set_color(0.6f, 0.1f, 1.0f);
      Vec3 k7_vec(k7x, k7y, 0.0); k7_vec.set_label("k7"); k7_vec.set_color(0.6f, 0.1f, 1.0f);
      k7_vec.set_semantic("Vector"); k7_vec.set_origin(k7_pt); k7_vec.set_class("rk_stage"); k7_vec.set_layer("k7");

      // 4th order step
      double dx4 = k1x * (5179.0/57600.0) + k3x * (7571.0/16695.0) + k4x * (393.0/640.0) - k5x * (92097.0/339200.0) + k6x * (187.0/2100.0) + k7x * (1.0/40.0);
      double dy4 = k1y * (5179.0/57600.0) + k3y * (7571.0/16695.0) + k4y * (393.0/640.0) - k5y * (92097.0/339200.0) + k6y * (187.0/2100.0) + k7y * (1.0/40.0);

      // 5th order step
      dx = k1x * (35.0/384.0) + k3x * (500.0/1113.0) + k4x * (125.0/192.0) - k5x * (2187.0/6784.0) + k6x * (11.0/84.0);
      dy = k1y * (35.0/384.0) + k3y * (500.0/1113.0) + k4y * (125.0/192.0) - k5y * (2187.0/6784.0) + k6y * (11.0/84.0);
      
      double err = std::sqrt((dx - dx4)*(dx - dx4) + (dy - dy4)*(dy - dy4));
      
      if (err > tolerance) {
        step_accepted = false;
      }
      
      double h_opt = current_h * 0.9 * std::pow(tolerance / (err + 1e-10), 0.2);
      h_opt = std::max(current_h * 0.2, std::min(current_h * 5.0, h_opt));
      
      if (!step_accepted) {
        iii::Recorder::get().record(iii::EventMessage{
          0, stepStr + ": Step Rejected",
          "Error (" + std::to_string(err) + ") > Tol (" + std::to_string((double)tolerance) + ")\n" +
          "Shrinking h to " + std::to_string(h_opt)
        });
        
        Vec3 rej_origin(x, y, 0.0);
        rej_origin.set_class("stage_point"); rej_origin.set_layer("rejected"); rej_origin.set_color(1.0f, 0.0f, 0.0f);
        Vec3 rej_vec(dx, dy, 0.0); rej_vec.set_label("Rejected dx"); rej_vec.set_color(1.0f, 0.0f, 0.0f);
        rej_vec.set_semantic("Vector"); rej_vec.set_origin(rej_origin); rej_vec.set_class("rk_result"); rej_vec.set_layer("rejected");
      }
      current_h = h_opt;
    }
    } while (!step_accepted);

    // Show the final step vector
    Vec3 step_origin(x, y, 0.0);
    step_origin.set_class("stage_point");
    step_origin.set_layer("result");
    step_origin.set_color(0.9f, 0.9f, 1.0f);

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
    trajectory.emplace_back(x, y, 0.0);
    auto& newPos = trajectory.back();
    newPos.set_label("x" + std::to_string(step + 1));
    newPos.set_color(1.0f, 1.0f, 1.0f);
    newPos.set_class("trajectory_point");
    newPos.set_layer("trajectory");

    // Join with previous point
    trajectory_lines.emplace_back(dx, dy, 0.0);
    auto& line = trajectory_lines.back();
    line.set_origin(trajectory[step]); // index 'step' is the previous point
    line.set_semantic("Vector");
    line.set_class("trajectory_segment");
    line.set_layer("trajectory");
    line.set_color(0.5f, 0.5f, 0.5f);
  }

  iii::Recorder::get().record(iii::EventMessage{
      0, "RK4 integration complete — 12 steps on spiral field",
      "// Toggle k1/k2/k3/k4 layers to see individual stages\n"
      "// Set 'rk_stage' class to Arrow for better visualization"});

  iii::Recorder::get().removeListener(listener);
  return listener->events;
}

} // namespace DemoCases
