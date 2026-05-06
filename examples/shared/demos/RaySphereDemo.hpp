#pragma once

#include "DemoUtils.hpp"
#include <cmath>
#include <iii/Parameter.hpp>
#include <iii/Recorder.hpp>
#include <iii/Vector.hpp>

namespace DemoCases {

using Vector3 = iii::Vector3<double, true>;

inline std::vector<iii::Event> generate_raysphere_demo() {
  auto *listener = new VectorListener();
  iii::Recorder::get().addListener(listener);
  iii::Recorder::get().record(
      iii::EventMessage{0, "Ray-Sphere Intersection Demo", "Analytic solver"});

  // Camera
  iii::EventSetCamera cam;
  cam.dist = 12.0f;
  cam.pitch = 0.3f;
  cam.yaw = 0.8f;
  cam.fov = 45.0f;
  iii::Recorder::get().record(cam);

  // Parameters
  iii::Parameter<double> sphere_radius("sphere_radius", 2.0);
  iii::Parameter<double> ray_origin_x("ray_origin_x", -5.0);
  iii::Parameter<double> sweep_range("sweep_range", 3.0);
  iii::Parameter<double> num_rays_p("num_rays", 30.0);
  int num_rays = std::max(5, std::min(100, (int)num_rays_p));

  double r = (double)sphere_radius;

  // --- Static Scene Setup ---
  // Sphere center
  Vector3 sphereCenter(0.0, 0.0, 0.0);
  sphereCenter.set_label("Sphere Center");
  sphereCenter.set_color(0.9f, 0.6f, 0.1f); // Orange
  sphereCenter.set_class("sphere_center");
  sphereCenter.set_visible(true);

  // Draw a wireframe circle to suggest the sphere (XY plane)
  // We approximate with line segments
  const int circleSegments = 32;
  std::vector<Vector3> circlePoints;
  for (int i = 0; i <= circleSegments; ++i) {
    double angle = 2.0 * M_PI * i / circleSegments;
    Vector3 cp(std::cos(angle) * r, std::sin(angle) * r, 0.0);
    cp.set_visible(false); // Points themselves hidden, only lines visible
    circlePoints.push_back(cp);
  }
  // Connect circle points with lines
  for (int i = 0; i < circleSegments; ++i) {
    iii::Recorder::get().record(iii::EventCreateRelation{
        iii::Recorder::get().nextId(),
        circlePoints[i].m_data.id,
        circlePoints[i + 1].m_data.id,
        {0.5f, 0.4f, 0.1f} // Dim orange
    });
  }

  // Also draw a circle in XZ plane for depth
  std::vector<Vector3> circlePointsXZ;
  for (int i = 0; i <= circleSegments; ++i) {
    double angle = 2.0 * M_PI * i / circleSegments;
    Vector3 cp(std::cos(angle) * r, 0.0, std::sin(angle) * r);
    cp.set_visible(false);
    circlePointsXZ.push_back(cp);
  }
  for (int i = 0; i < circleSegments; ++i) {
    iii::Recorder::get().record(iii::EventCreateRelation{
        iii::Recorder::get().nextId(),
        circlePointsXZ[i].m_data.id,
        circlePointsXZ[i + 1].m_data.id,
        {0.4f, 0.3f, 0.1f}
    });
  }

  // --- Sweep rays across the sphere ---
  for (int ri = 0; ri < num_rays; ++ri) {
    double frac = (double)ri / (num_rays - 1); // 0..1
    double sweep_y = -sweep_range + 2.0 * sweep_range * frac;

    iii::Recorder::get().record(iii::EventMessage{
        0, "=== Ray " + std::to_string(ri + 1) + "/" + std::to_string(num_rays) + " ===",
        "sweep_y = " + std::to_string(sweep_y)});

    // Ray origin
    Vector3 O((double)ray_origin_x, sweep_y, 0.0);
    O.set_label("O");
    O.set_color(0.2f, 0.7f, 1.0f); // Light blue
    O.set_visible(true);

    // Ray direction (pointing toward +X, toward sphere)
    Vector3 D(1.0, 0.0, 0.0);
    D.set_label("D");
    D.set_color(0.2f, 0.7f, 1.0f);
    D.set_semantic("Vector");
    D.set_origin(O);
    D.set_visible(true);

    // Step 1: Compute L = C - O
    iii::Recorder::get().record(
        iii::EventMessage{0, "Step 1: L = C - O", "Vector from ray origin to sphere center"});
    Vector3 L = sphereCenter - O;
    L.set_label("L");
    L.set_color(1.0f, 1.0f, 0.4f); // Yellow
    L.set_semantic("Vector");
    L.set_origin(O);
    L.set_visible(true);

    // Step 2: tca = L · D (projection onto ray)
    double tca = L.dot(D);
    iii::Recorder::get().record(iii::EventMessage{
        0, "Step 2: tca = L . D = " + std::to_string(tca),
        "Projection of L onto ray direction"});

    // Show the projection point
    Vector3 projPoint(O.x() + tca * D.x(), O.y() + tca * D.y(), O.z() + tca * D.z());
    projPoint.set_label("Proj");
    projPoint.set_color(1.0f, 1.0f, 0.0f); // Bright yellow
    projPoint.set_visible(true);

    // Line from O to projection point
    iii::Recorder::get().record(iii::EventCreateRelation{
        iii::Recorder::get().nextId(),
        O.m_data.id,
        projPoint.m_data.id,
        {0.3f, 0.5f, 0.7f} // Muted blue
    });

    // Step 3: d² = L·L - tca²
    double L_dot_L = L.dot(L);
    double d2 = L_dot_L - tca * tca;
    iii::Recorder::get().record(iii::EventMessage{
        0, "Step 3: d² = |L|² - tca² = " + std::to_string(d2),
        "Squared closest approach distance"});

    // Line from projection point to sphere center (shows closest approach)
    iii::Recorder::get().record(iii::EventCreateRelation{
        iii::Recorder::get().nextId(),
        projPoint.m_data.id,
        sphereCenter.m_data.id,
        {0.8f, 0.6f, 0.2f} // Orange dashed
    });

    // Step 4: Discriminant check
    double r2 = r * r;
    bool hit = (d2 <= r2);

    if (!hit) {
      iii::Recorder::get().record(iii::EventMessage{
          0, "Step 4: d² > r² → MISS!",
          "d²=" + std::to_string(d2) + " > r²=" + std::to_string(r2)});

      // Draw the full ray as a miss (dim red)
      Vector3 rayEnd(O.x() + 12.0 * D.x(), O.y() + 12.0 * D.y(), O.z() + 12.0 * D.z());
      rayEnd.set_visible(false);
      iii::Recorder::get().record(iii::EventCreateRelation{
          iii::Recorder::get().nextId(),
          O.m_data.id,
          rayEnd.m_data.id,
          {0.5f, 0.2f, 0.2f} // Dim red = miss
      });

      // Hide temporaries
      L.set_visible(false);
      projPoint.set_visible(false);
      O.set_visible(false);
      D.set_visible(false);

    } else {
      // Step 5: Compute intersection distances
      double thc = std::sqrt(r2 - d2);
      double t0 = tca - thc;
      double t1 = tca + thc;

      iii::Recorder::get().record(iii::EventMessage{
          0, "Step 4: d² <= r² → HIT!",
          "thc=" + std::to_string(thc) + ", t0=" + std::to_string(t0) + ", t1=" + std::to_string(t1)});

      // Step 6: Hit points
      iii::Recorder::get().record(
          iii::EventMessage{0, "Step 5: Hit Points", "P = O + t * D"});

      Vector3 P0(O.x() + t0 * D.x(), O.y() + t0 * D.y(), O.z() + t0 * D.z());
      P0.set_label("P_enter");
      P0.set_color(0.0f, 1.0f, 0.3f); // Green
      P0.set_class("hit_point");
      P0.set_visible(true);

      Vector3 P1(O.x() + t1 * D.x(), O.y() + t1 * D.y(), O.z() + t1 * D.z());
      P1.set_label("P_exit");
      P1.set_color(1.0f, 0.3f, 0.0f); // Red-orange
      P1.set_class("hit_point");
      P1.set_visible(true);

      // Ray segment: O → P0 (incoming)
      iii::Recorder::get().record(iii::EventCreateRelation{
          iii::Recorder::get().nextId(),
          O.m_data.id,
          P0.m_data.id,
          {0.2f, 0.8f, 0.3f} // Green
      });

      // Ray segment: P0 → P1 (inside sphere)
      iii::Recorder::get().record(iii::EventCreateRelation{
          iii::Recorder::get().nextId(),
          P0.m_data.id,
          P1.m_data.id,
          {1.0f, 0.5f, 0.0f} // Orange (inside sphere)
      });

      // Step 7: Surface normal at entry point
      iii::Recorder::get().record(
          iii::EventMessage{0, "Step 6: Surface Normal", "N = normalize(P - C)"});

      Vector3 N0_raw = P0 - sphereCenter;
      double n0_len = std::sqrt(N0_raw.x() * N0_raw.x() + N0_raw.y() * N0_raw.y() + N0_raw.z() * N0_raw.z());
      Vector3 N0(N0_raw.x() / n0_len, N0_raw.y() / n0_len, N0_raw.z() / n0_len);
      N0.set_label("N");
      N0.set_color(0.4f, 1.0f, 0.4f); // Light green
      N0.set_semantic("Vector");
      N0.set_origin(P0);
      N0.set_visible(true);

      // Step 8: Reflected ray
      iii::Recorder::get().record(
          iii::EventMessage{0, "Step 7: Reflected Ray", "R = D - 2(D·N)N"});

      double d_dot_n = D.dot(N0);
      Vector3 reflectTerm = N0 * (2.0 * d_dot_n);
      Vector3 R = D - reflectTerm;
      R.set_label("R");
      R.set_color(1.0f, 0.2f, 0.8f); // Magenta
      R.set_semantic("Vector");
      R.set_origin(P0);
      R.set_visible(true);

      // Draw reflected ray as an extended line
      Vector3 reflEnd(P0.x() + R.x() * 4.0, P0.y() + R.y() * 4.0, P0.z() + R.z() * 4.0);
      reflEnd.set_visible(false);
      iii::Recorder::get().record(iii::EventCreateRelation{
          iii::Recorder::get().nextId(),
          P0.m_data.id,
          reflEnd.m_data.id,
          {1.0f, 0.2f, 0.8f} // Magenta
      });

      // Hide temporaries for this ray before next iteration
      L.set_visible(false);
      projPoint.set_visible(false);
      O.set_visible(false);
      D.set_visible(false);
      N0.set_visible(false);
      R.set_visible(false);
      // Keep P0, P1, and lines visible to accumulate the result
    }
  }

  iii::Recorder::get().record(
      iii::EventMessage{0, "Sweep Complete", "All rays processed"});

  iii::Recorder::get().removeListener(listener);
  return listener->events;
}

} // namespace DemoCases
