#pragma once

#include "DemoUtils.hpp"
#include <array>
#include <cmath>
#include <iii/Parameter.hpp>
#include <iii/Recorder.hpp>
#include <iii/Vector.hpp>
#include <vector>

namespace DemoCases {

using Vector3 = iii::Vector3<double, true>;

inline std::vector<iii::Event> generate_ik_demo() {
  auto *listener = new VectorListener();
  iii::Recorder::get().addListener(listener);
  iii::Recorder::get().record(
      iii::EventMessage{0, "Initializing IK Demo...", "CCD Solver"});

  // Camera settings
  iii::EventSetCamera cam;
  cam.dist = 12.0f;
  cam.pitch = 0.5f;
  cam.yaw = 0.8f;
  cam.fov = 45.0f;
  iii::Recorder::get().record(cam);

  // Parameters
  iii::Parameter<double> target_x("target_x", 3.0);
  iii::Parameter<double> target_y("target_y", 4.0);
  iii::Parameter<double> target_z("target_z", 0.0);
  iii::Parameter<double> num_segments_p("segments", 3.0);
  int num_segments = (int)num_segments_p;
  if (num_segments < 1)
    num_segments = 1;
  if (num_segments > 10)
    num_segments = 10;

  // Visual Setup
  // Target Marker
  Vector3 target((double)target_x, (double)target_y, (double)target_z);
  target.set_label("Target");
  target.set_color(1.0, 0.0, 0.0); // Red
  target.set_visible(true);

  // Arm Segments (Points representing joints/ends)
  std::vector<Vector3> joints;
  double seg_len = 2.0;

  // Initialize straight up
  for (int i = 0; i <= num_segments; ++i) {
    Vector3 j(0, i * seg_len, 0);
    j.set_label("Joint " + std::to_string(i));
    j.set_color(0.0, 1.0, 0.0);
    j.set_visible(true);
    joints.push_back(j);
  }

  // Visual Lines connecting joints
  for (size_t i = 0; i < joints.size() - 1; ++i) {
    iii::Recorder::get().record(iii::EventCreateRelation{
        iii::Recorder::get().nextId(),
        joints[i].m_data.id,
        joints[i + 1].m_data.id,
        {1.0f, 1.0f, 1.0f} // White lines
    });
  }

  // CCD Solver
  using VecMath = Vector3::Base; // Use pure Eigen vector for math

  int max_iter = 10;
  for (int iter = 0; iter < max_iter; ++iter) {
    // Loop from end effector to base
    for (int i = num_segments - 1; i >= 0; --i) {
      // Current joint (pivot)
      const VecMath r_vec = joints[i];
      // End effector
      const VecMath e_vec = joints[num_segments];
      // Target
      const VecMath t_vec = target;

      VecMath root_to_end = e_vec - r_vec;
      VecMath root_to_target = t_vec - r_vec;

      double len_re = root_to_end.norm();
      double len_rt = root_to_target.norm();

      if (len_re < 1e-6 || len_rt < 1e-6)
        continue;

      // Normalize
      root_to_end /= len_re;
      root_to_target /= len_rt;

      // Rotation needed
      VecMath axis = root_to_end.cross(root_to_target);
      double cos_theta = root_to_end.dot(root_to_target);

      // Clamp
      if (cos_theta > 1.0)
        cos_theta = 1.0;
      if (cos_theta < -1.0)
        cos_theta = -1.0;

      double theta = std::acos(cos_theta);

      if (std::abs(theta) < 1e-4)
        continue;

      // Damping
      if (theta > 0.5)
        theta = 0.5;

      // Apply rotation
      double len_axis = axis.norm();
      if (len_axis < 1e-6)
        continue;

      VecMath k = axis / len_axis;

      // Create rotation (using AngleAxis for clarity)
      Eigen::AngleAxisd rotation(theta, k);

      // Rotate all subsequent joints
      for (int j = i + 1; j <= num_segments; ++j) {
        VecMath p_current = joints[j];
        VecMath v = p_current - r_vec;
        VecMath v_rot = rotation * v;

        // Assign back to Visual Vector: implicit record_move()!
        joints[j] = r_vec + v_rot;
      }
    }
    iii::Recorder::get().record(iii::EventMessage{
        0, "Solver Step", "Iteration " + std::to_string(iter)});
  }

  iii::Recorder::get().removeListener(listener);
  return listener->events;
}

} // namespace DemoCases
