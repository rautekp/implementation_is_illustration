#pragma once

#include <string>
#include <variant>
#include <vector>

namespace iii {

struct Color {
  float r, g, b;
};

// Simple event types for the PoC
struct EventCreate {
  size_t id;
  std::string label;
  Color color;
  // Initial position/value could be added here
};

struct EventMove {
  size_t id;
  double x, y, z; // Assuming 3D for now
};

struct EventBinaryOp {
  size_t result_id;
  size_t operand1_id;
  size_t operand2_id;
  std::string operation; // "+", "-", etc.
};

struct EventScalarOp {
  size_t result_id;
  size_t operand_id;
  double scalar;
  std::string operation; // "*", "/", etc.
};

struct EventFunctionStart {
  std::string name;
};

struct EventFunctionEnd {};

struct EventDestroy {
  size_t id;
};

struct EventSetSemantic {
  size_t id;
  std::string semantic;
};

struct EventSetOrigin {
  size_t id;
  size_t origin_id;
};

struct EventSetVisible {
  size_t id;
  bool visible;
};

struct EventSetColor {
  size_t id;
  Color color;
};

using Event = std::variant<EventCreate, EventMove, EventBinaryOp, EventScalarOp,
                           EventFunctionStart, EventFunctionEnd, EventDestroy,
                           EventSetSemantic, EventSetOrigin, EventSetVisible,
                           EventSetColor>;

struct IEventListener; // Forward declaration

class Recorder {
public:
  static Recorder &get() {
    static Recorder instance;
    return instance;
  }

  // Visual methods
  void record(Event e);
  void function_start(std::string name);
  void function_end();
  void dump(const std::string &filename);

  void addListener(IEventListener *listener);

  size_t nextId() { return m_nextId++; }

private:
  Recorder() = default;

  std::vector<Event> m_events;
  std::vector<IEventListener *> m_listeners;
  size_t m_nextId = 1;
};

} // namespace iii
