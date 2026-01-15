#pragma once

#include "DemoUtils.hpp"
#include <cmath>
#include <iii/Parameter.hpp>
#include <iii/Recorder.hpp>
#include <iii/Vector.hpp>


namespace DemoCases {

using Vector3 = iii::Vector3<double, true>;

inline std::vector<iii::Event> generate_solar_system() {
  auto *listener = new VectorListener();
  iii::Recorder::get().addListener(listener);
  iii::Recorder::get().record(
      iii::EventMessage{0, "Initializing Solar System...", ""});

  // Camera
  iii::EventSetCamera cam;
  cam.dist = 15.0f;
  cam.pitch = 0.8f;
  cam.yaw = 0.0f;
  cam.fov = 45.0f;
  iii::Recorder::get().record(cam);

  Vector3 sun(0, 0, 0);
  sun.set_label("Sun");
  sun.set_color(1.0, 1.0, 0.0); // Yellow
  sun.set_visible(true);

  Vector3 earth(0, 0, 0);
  earth.set_label("Earth");
  earth.set_color(0.0, 0.5, 1.0); // Blue
  earth.set_visible(true);

  Vector3 moon(0, 0, 0);
  moon.set_label("Moon");
  moon.set_color(0.8, 0.8, 0.8); // Grey
  moon.set_visible(true);

  // Interactive Parameter
  iii::Parameter<double> dt("dt", 0.05);

  for (int i = 0; i < 500; ++i) {
    double t = i * dt;

    // Sun static (no move needed)
    // sun.record_move(); // Optional if it moved

    // Earth orbits Sun
    double earth_r = 5.0;
    double earth_x = std::cos(t) * earth_r;
    double earth_z = std::sin(t) * earth_r;

    // Update in place to preserve Color/Label/ID
    earth[0] = earth_x;
    earth[1] = 0.0;
    earth[2] = earth_z;
    earth.record_move();

    iii::Recorder::get().record(
        iii::EventMessage{0, "Orbiting...", "Earth orbits Sun"});

    // Moon orbits Earth
    double moon_r = 1.0;
    double moon_t = t * 3.0; // Faster
    double moon_x = earth_x + std::cos(moon_t) * moon_r;
    double moon_z = earth_z + std::sin(moon_t) * moon_r;

    moon[0] = moon_x;
    moon[1] = 0.0;
    moon[2] = moon_z;
    moon.record_move();
  }

  iii::Recorder::get().removeListener(listener);
  return listener->events;
}

} // namespace DemoCases
