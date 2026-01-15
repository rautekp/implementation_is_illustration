#pragma once

#include <iii/IEventListener.hpp>
#include <vector>

namespace DemoCases {

// Helper listener to capture events
struct VectorListener : public iii::IEventListener {
  std::vector<iii::Event> events;
  void onEvent(const iii::Event &e) override { events.push_back(e); }
};

} // namespace DemoCases
