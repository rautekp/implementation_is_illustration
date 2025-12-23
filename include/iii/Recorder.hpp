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

struct EventDotOp {
  size_t operand1_id;
  size_t operand2_id;
  double result_scalar;
};

// Matrix Events
struct EventSetMatrix {
  size_t id;
  // Column-major or row-major? Eigen is col-major by default but stores usually
  // contiguous. 16 floats.
  double m[16];
};

struct EventTransform {
  size_t result_id;
  size_t matrix_id;
  size_t vector_id;
};

struct EventMessage {
  size_t id;
  std::string message;
  std::string code;
};

struct EventSetLabel {
  size_t id;
  std::string label;
};

// Camera Control
struct EventSetCamera {
  size_t id = 0;
  float pitch = 0.5f, yaw = 0.5f, dist = 5.0f;
  float fov = 45.0f;
};

using Event =
    std::variant<EventCreate, EventMove, EventBinaryOp, EventScalarOp,
                 EventFunctionStart, EventFunctionEnd, EventDestroy,
                 EventSetSemantic, EventSetOrigin, EventSetVisible,
                 EventSetColor, EventDotOp, EventSetMatrix, EventTransform,
                 EventMessage, EventSetCamera, EventSetLabel>;

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
  void removeListener(IEventListener *listener);

  size_t nextId() { return m_nextId++; }

private:
  Recorder() = default;

  std::vector<Event> m_events;
  std::vector<IEventListener *> m_listeners;
  size_t m_nextId = 1;
};

} // namespace iii
