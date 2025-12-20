#pragma once

#include "iii/Recorder.hpp"
#include <string>

namespace iii {

struct IEventListener {
  virtual ~IEventListener() = default;

  // Called when a new event happens
  virtual void onEvent(const Event &e) = 0;
};

} // namespace iii
