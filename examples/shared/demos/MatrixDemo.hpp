#pragma once

#include "DemoUtils.hpp"
#include <cmath>
#include <iii/Matrix.hpp>
#include <iii/Recorder.hpp>


namespace DemoCases {

using Vector3 = iii::Vector3<double, true>;
using Matrix4 = iii::Matrix4<double, true>;

inline std::vector<iii::Event> generate_matrix_demo() {
  // Setup listener
  auto *listener = new VectorListener();
  iii::Recorder::get().addListener(listener);

  // --- Logic ---
  iii::Recorder::get().record(
      iii::EventMessage{0, "Initializing Matrix Demo...", ""});

  // Camera
  iii::EventSetCamera camParams;
  camParams.id = 0;
  camParams.pitch = 0.5f;
  camParams.yaw = 0.5f;
  camParams.dist = 7.0f;
  camParams.fov = 45.0f;
  iii::Recorder::get().record(camParams);

  Matrix4 worldFrame = Matrix4::Identity();
  worldFrame.set_label("World");

  iii::Recorder::get().record(iii::EventMessage{
      0, "Creating Moving Frame...", "Matrix4 moving = Identity();"});
  Matrix4 movingFrame = Matrix4::Identity();
  movingFrame.set_label("Moving Frame");

  Vector3 localPoint(0.5, 0.5, 0.0);
  localPoint.set_label("P_local");
  localPoint.set_color(1.0, 1.0, 0.0);

  for (int i = 0; i < 300; ++i) { // 300 Frames
    double t = i * 0.05;
    double angle = t;
    double x_pos = std::sin(t) * 1.5;

    iii::Recorder::get().record(iii::EventMessage{
        0, "Transforming Frame...", "M = Translate(x) * Rotate(ang);"});
    movingFrame = Matrix4::Translate(x_pos, 0.0, 0.0) *
                  Matrix4::Rotate(angle, Vector3(0, 1, 0));

    iii::Recorder::get().record(iii::EventMessage{0, "Transforming Point...",
                                                  "P_world = M * P_local;"});
    Vector3 worldPoint = movingFrame * localPoint;
    worldPoint.set_label("P_world");
    worldPoint.set_color(0.0, 1.0, 1.0);
    worldPoint.set_visible(true);
  }

  iii::Recorder::get().removeListener(listener); // Clean up
  return listener->events;
}

} // namespace DemoCases
