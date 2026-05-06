#pragma once

#include "DemoUtils.hpp"
#include <cmath>
#include <iii/Parameter.hpp>
#include <iii/Recorder.hpp>
#include <iii/Vector.hpp>
#include <vector>

namespace DemoCases {

using Vector3 = iii::Vector3<double, true>;

// ============================================================
// Helper: Rodrigues rotation of vector v around axis k by angle theta
// ============================================================
inline void rodrigues(double vx, double vy, double vz,
                      double kx, double ky, double kz, double theta,
                      double &ox, double &oy, double &oz) {
  double ct = std::cos(theta);
  double st = std::sin(theta);
  double dot = vx * kx + vy * ky + vz * kz;
  // v*cos(t) + (k x v)*sin(t) + k*(k.v)*(1 - cos(t))
  ox = vx * ct + (ky * vz - kz * vy) * st + kx * dot * (1.0 - ct);
  oy = vy * ct + (kz * vx - kx * vz) * st + ky * dot * (1.0 - ct);
  oz = vz * ct + (kx * vy - ky * vx) * st + kz * dot * (1.0 - ct);
}

inline std::vector<iii::Event> generate_curve_frame_demo() {
  auto *listener = new VectorListener();
  iii::Recorder::get().addListener(listener);
  iii::Recorder::get().record(
      iii::EventMessage{0, "Curve Frame Demo", ""});

  // Camera
  iii::EventSetCamera cam;
  cam.dist = 14.0f;
  cam.pitch = 0.5f;
  cam.yaw = 0.4f;
  cam.fov = 45.0f;
  iii::Recorder::get().record(cam);

  // Parameters
  iii::Parameter<double> helix_radius("helix_radius", 3.0);
  iii::Parameter<double> helix_pitch("helix_pitch", 0.8);
  iii::Parameter<double> num_frames_p("num_frames", 20.0);
  iii::Parameter<double> frame_scale_p("frame_scale", 1.0);
  iii::Parameter<double> frame_type_p("frame_type (0=Frenet, 1=Bishop)", 0.0);

  double R = (double)helix_radius;
  double P = (double)helix_pitch;
  int num_frames = std::max(3, std::min(50, (int)num_frames_p));
  double frame_scale = (double)frame_scale_p;
  int frame_type = (int)std::round((double)frame_type_p);
  if (frame_type < 0) frame_type = 0;
  if (frame_type > 1) frame_type = 1;

  double t_max = 4.0 * M_PI; // Two full turns
  int num_curve_pts = 200;

  std::string method_name = (frame_type == 0) ? "Frenet-Serret" : "Bishop (Rotation-Minimizing)";
  iii::Recorder::get().record(iii::EventMessage{
      0, "Method: " + method_name,
      "Helix: r(t) = (R*cos(t), P*t, R*sin(t))"});

  // ============================================================
  // Draw the curve
  // ============================================================
  std::vector<Vector3> curvePts;
  for (int i = 0; i <= num_curve_pts; ++i) {
    double t = t_max * i / num_curve_pts;
    Vector3 pt(R * std::cos(t), P * t, R * std::sin(t));
    pt.set_visible(false);
    pt.set_layer("curve");
    curvePts.push_back(pt);
  }
  for (int i = 0; i < num_curve_pts; ++i) {
    iii::Recorder::get().record(iii::EventCreateRelation{
        iii::Recorder::get().nextId(),
        curvePts[i].m_data.id,
        curvePts[i + 1].m_data.id,
        {0.3f, 0.5f, 0.8f}
    });
  }

  // ============================================================
  // Precompute tangents at frame sample points
  // ============================================================
  struct FrameSample {
    double t;
    double px, py, pz; // position
    double tx, ty, tz; // unit tangent
  };
  std::vector<FrameSample> samples(num_frames);

  for (int fi = 0; fi < num_frames; ++fi) {
    double frac = (double)fi / (num_frames - 1);
    double t = t_max * frac;
    samples[fi].t = t;
    samples[fi].px = R * std::cos(t);
    samples[fi].py = P * t;
    samples[fi].pz = R * std::sin(t);

    // r'(t) = (-R*sin(t), P, R*cos(t))
    double dx = -R * std::sin(t);
    double dy = P;
    double dz = R * std::cos(t);
    double len = std::sqrt(dx * dx + dy * dy + dz * dz);
    samples[fi].tx = dx / len;
    samples[fi].ty = dy / len;
    samples[fi].tz = dz / len;
  }

  // ============================================================
  // Bishop frame: precompute all N,B via parallel transport
  // ============================================================
  struct Frame {
    double nx, ny, nz;
    double bx, by, bz;
  };
  std::vector<Frame> bishop_frames(num_frames);

  if (frame_type == 1) {
    // Initial N: arbitrary vector perpendicular to T0
    double tx0 = samples[0].tx, ty0 = samples[0].ty, tz0 = samples[0].tz;
    double nx, ny, nz;
    // Pick the axis least aligned with T to cross with
    if (std::abs(tx0) < std::abs(ty0) && std::abs(tx0) < std::abs(tz0)) {
      // Cross with X axis
      nx = 0.0; ny = -tz0; nz = ty0;
    } else if (std::abs(ty0) < std::abs(tz0)) {
      nx = tz0; ny = 0.0; nz = -tx0;
    } else {
      nx = -ty0; ny = tx0; nz = 0.0;
    }
    double nlen = std::sqrt(nx * nx + ny * ny + nz * nz);
    nx /= nlen; ny /= nlen; nz /= nlen;

    bishop_frames[0].nx = nx;
    bishop_frames[0].ny = ny;
    bishop_frames[0].nz = nz;
    bishop_frames[0].bx = ty0 * nz - tz0 * ny;
    bishop_frames[0].by = tz0 * nx - tx0 * nz;
    bishop_frames[0].bz = tx0 * ny - ty0 * nx;

    // Parallel transport using rotation from T_i to T_{i+1}
    for (int fi = 1; fi < num_frames; ++fi) {
      double t0x = samples[fi - 1].tx, t0y = samples[fi - 1].ty, t0z = samples[fi - 1].tz;
      double t1x = samples[fi].tx, t1y = samples[fi].ty, t1z = samples[fi].tz;

      // Rotation axis = T_i x T_{i+1}
      double ax = t0y * t1z - t0z * t1y;
      double ay = t0z * t1x - t0x * t1z;
      double az = t0x * t1y - t0y * t1x;
      double alen = std::sqrt(ax * ax + ay * ay + az * az);

      double prev_nx = bishop_frames[fi - 1].nx;
      double prev_ny = bishop_frames[fi - 1].ny;
      double prev_nz = bishop_frames[fi - 1].nz;

      if (alen < 1e-10) {
        // Tangents nearly identical — no rotation needed
        bishop_frames[fi].nx = prev_nx;
        bishop_frames[fi].ny = prev_ny;
        bishop_frames[fi].nz = prev_nz;
      } else {
        ax /= alen; ay /= alen; az /= alen;
        double dot = t0x * t1x + t0y * t1y + t0z * t1z;
        if (dot > 1.0) dot = 1.0;
        if (dot < -1.0) dot = -1.0;
        double theta = std::acos(dot);

        rodrigues(prev_nx, prev_ny, prev_nz, ax, ay, az, theta,
                  bishop_frames[fi].nx, bishop_frames[fi].ny, bishop_frames[fi].nz);
      }
      // B = T x N
      bishop_frames[fi].bx = t1y * bishop_frames[fi].nz - t1z * bishop_frames[fi].ny;
      bishop_frames[fi].by = t1z * bishop_frames[fi].nx - t1x * bishop_frames[fi].nz;
      bishop_frames[fi].bz = t1x * bishop_frames[fi].ny - t1y * bishop_frames[fi].nx;
    }
  }

  // ============================================================
  // Emit frames with step-by-step events
  // ============================================================
  for (int fi = 0; fi < num_frames; ++fi) {
    double t = samples[fi].t;

    iii::Recorder::get().record(iii::EventMessage{
        0, "=== Frame " + std::to_string(fi + 1) + "/" + std::to_string(num_frames) + " ===",
        "t = " + std::to_string(t)});

    // Point on curve
    Vector3 pos(samples[fi].px, samples[fi].py, samples[fi].pz);
    pos.set_label("r(t)");
    pos.set_color(1.0f, 1.0f, 1.0f);
    pos.set_class("curve_point");
    pos.set_visible(true);

    double Tx = samples[fi].tx, Ty = samples[fi].ty, Tz = samples[fi].tz;

    // --- Tangent (same for both methods) ---
    iii::Recorder::get().record(iii::EventMessage{
        0, "Tangent T = normalize(r'(t))", ""});

    Vector3 T(Tx * frame_scale, Ty * frame_scale, Tz * frame_scale);
    T.set_label("T");
    T.set_color(1.0f, 0.2f, 0.2f);
    T.set_semantic("Vector");
    T.set_origin(pos);
    T.set_class("frame_vector");
    T.set_layer("tangent");
    T.set_visible(true);

    double Nx, Ny, Nz, Bx, By, Bz;

    if (frame_type == 0) {
      // ============================================================
      // FRENET-SERRET
      // ============================================================
      // r''(t) = (-R*cos(t), 0, -R*sin(t))
      double ddx = -R * std::cos(t);
      double ddy = 0.0;
      double ddz = -R * std::sin(t);

      iii::Recorder::get().record(iii::EventMessage{
          0, "Frenet: r''(t) = (-R*cos(t), 0, -R*sin(t))", ""});

      Vector3 r_pp(ddx, ddy, ddz);
      r_pp.set_label("r''");
      r_pp.set_color(0.5f, 0.5f, 0.3f);
      r_pp.set_semantic("Vector");
      r_pp.set_origin(pos);
      r_pp.set_layer("intermediate");
      r_pp.set_visible(true);

      // N = normalize(r'' - (r''·T)T)
      double rpp_dot_T = ddx * Tx + ddy * Ty + ddz * Tz;
      double px = ddx - rpp_dot_T * Tx;
      double py = ddy - rpp_dot_T * Ty;
      double pz = ddz - rpp_dot_T * Tz;
      double plen = std::sqrt(px * px + py * py + pz * pz);

      iii::Recorder::get().record(iii::EventMessage{
          0, "N = normalize(r'' - (r''.T)T)",
          "Remove tangential component"});

      if (plen > 1e-10) {
        Nx = px / plen; Ny = py / plen; Nz = pz / plen;
      } else {
        Nx = 0; Ny = 1; Nz = 0; // Fallback
      }

      // B = T x N
      Bx = Ty * Nz - Tz * Ny;
      By = Tz * Nx - Tx * Nz;
      Bz = Tx * Ny - Ty * Nx;

      iii::Recorder::get().record(iii::EventMessage{
          0, "B = T x N", "Cross product completes frame"});

      r_pp.set_visible(false);

    } else {
      // ============================================================
      // BISHOP (ROTATION-MINIMIZING)
      // ============================================================
      Nx = bishop_frames[fi].nx;
      Ny = bishop_frames[fi].ny;
      Nz = bishop_frames[fi].nz;
      Bx = bishop_frames[fi].bx;
      By = bishop_frames[fi].by;
      Bz = bishop_frames[fi].bz;

      if (fi == 0) {
        iii::Recorder::get().record(iii::EventMessage{
            0, "Bishop: Choose initial N perp to T",
            "Arbitrary perpendicular vector"});
      } else {
        iii::Recorder::get().record(iii::EventMessage{
            0, "Bishop: Parallel transport N from previous frame",
            "Rotate N by angle between T_{i-1} and T_i"});
      }

      iii::Recorder::get().record(iii::EventMessage{
          0, "B = T x N", "Cross product completes frame"});
    }

    // --- Display N ---
    Vector3 N(Nx * frame_scale, Ny * frame_scale, Nz * frame_scale);
    N.set_label("N");
    N.set_color(0.2f, 1.0f, 0.2f);
    N.set_semantic("Vector");
    N.set_origin(pos);
    N.set_class("frame_vector");
    N.set_layer("normal");
    N.set_visible(true);

    // --- Display B ---
    Vector3 B(Bx * frame_scale, By * frame_scale, Bz * frame_scale);
    B.set_label("B");
    B.set_color(0.2f, 0.4f, 1.0f);
    B.set_semantic("Vector");
    B.set_origin(pos);
    B.set_class("frame_vector");
    B.set_layer("binormal");
    B.set_visible(true);
  }

  iii::Recorder::get().record(iii::EventMessage{
      0, method_name + " — Complete",
      std::to_string(num_frames) + " frames computed"});

  iii::Recorder::get().removeListener(listener);
  return listener->events;
}

} // namespace DemoCases
