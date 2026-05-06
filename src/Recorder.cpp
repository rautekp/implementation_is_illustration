#include "iii/Recorder.hpp"
#include "iii/IEventListener.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>

// Note: We might need to fetch nlohmann/json or write a simple manual dumper
// for dependency simplicity. For this PoC, let's write a manual simple JSON
// dumper to avoid external dep complications unless we add it to CMake. Let's
// stick to manual string formatting for zero-dependency PoC.

namespace iii {

void Recorder::addListener(IEventListener *listener) {
  m_listeners.push_back(listener);
}

void Recorder::removeListener(IEventListener *listener) {
  auto it = std::remove(m_listeners.begin(), m_listeners.end(), listener);
  m_listeners.erase(it, m_listeners.end());
}

void Recorder::record(Event e) {
  // Dispatch to listeners
  for (auto *l : m_listeners) {
    l->onEvent(e);
  }
  // Store for dump
  m_events.push_back(std::move(e));
}

void Recorder::function_start(std::string name) {
  record(EventFunctionStart{std::move(name)});
}

void Recorder::function_end() { record(EventFunctionEnd{}); }

void Recorder::dump(const std::string &filename) {
  std::ofstream out(filename);
  out << "[\n";

  for (size_t i = 0; i < m_events.size(); ++i) {
    const auto &event = m_events[i];

    std::visit(
        [&](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          out << "  { \"type\": ";
          if constexpr (std::is_same_v<T, EventCreate>) {
            out << "\"create\", \"id\": " << arg.id << ", \"label\": \""
                << arg.label << "\""
                << ", \"color\": [" << arg.color.r << "," << arg.color.g << ","
                << arg.color.b << "]";
          } else if constexpr (std::is_same_v<T, EventMove>) {
            out << "\"move\", \"id\": " << arg.id << ", \"pos\": [" << arg.x
                << "," << arg.y << "," << arg.z << "]";
          } else if constexpr (std::is_same_v<T, EventBinaryOp>) {
            out << "\"op\", \"res\": " << arg.result_id
                << ", \"op1\": " << arg.operand1_id
                << ", \"op2\": " << arg.operand2_id << ", \"symbol\": \""
                << arg.operation << "\"";
          } else if constexpr (std::is_same_v<T, EventScalarOp>) {
            out << "\"scalar_op\", \"res\": " << arg.result_id
                << ", \"op1\": " << arg.operand_id
                << ", \"scalar\": " << arg.scalar << ", \"symbol\": \""
                << arg.operation << "\"";
          } else if constexpr (std::is_same_v<T, EventFunctionStart>) {
            out << "\"function_start\", \"name\": \"" << arg.name << "\"";
          } else if constexpr (std::is_same_v<T, EventFunctionEnd>) {
            out << "\"function_end\"";
          } else if constexpr (std::is_same_v<T, EventDestroy>) {
            out << "\"destroy\", \"id\": " << arg.id;
          } else if constexpr (std::is_same_v<T, EventSetSemantic>) {
            out << "\"set_semantic\", \"id\": " << arg.id
                << ", \"semantic\": \"" << arg.semantic << "\"";
          } else if constexpr (std::is_same_v<T, EventSetOrigin>) {
            out << "\"set_origin\", \"id\": " << arg.id
                << ", \"origin_id\": " << arg.origin_id;
          } else if constexpr (std::is_same_v<T, EventSetVisible>) {
            out << "\"set_visible\", \"id\": " << arg.id
                << ", \"visible\": " << (arg.visible ? "true" : "false");
          } else if constexpr (std::is_same_v<T, EventSetColor>) {
            out << "\"set_color\", \"id\": " << arg.id << ", \"color\": ["
                << arg.color.r << "," << arg.color.g << "," << arg.color.b
                << "]";
          } else if constexpr (std::is_same_v<T, EventDotOp>) {
            out << "\"dot_op\", \"op1\": " << arg.operand1_id
                << ", \"op2\": " << arg.operand2_id
                << ", \"res\": " << arg.result_scalar;
          } else if constexpr (std::is_same_v<T, EventSetMatrix>) {
            out << "\"set_matrix\", \"id\": " << arg.id << ", \"m\": [";
            for (int k = 0; k < 16; ++k)
              out << arg.m[k] << (k < 15 ? "," : "");
            out << "]";
          } else if constexpr (std::is_same_v<T, EventTransform>) {
            out << "\"transform\", \"res\": " << arg.result_id
                << ", \"mat\": " << arg.matrix_id
                << ", \"vec\": " << arg.vector_id;
          } else if constexpr (std::is_same_v<T, EventMessage>) {
            out << "\"message\", \"id\": " << arg.id << ", \"msg\": \""
                << arg.message << "\""
                << ", \"code\": \"" << arg.code << "\"";
          } else if constexpr (std::is_same_v<T, EventSetCamera>) {
            out << "\"set_camera\", ";
            out << "\"pitch\": " << arg.pitch << ", ";
            out << "\"yaw\": " << arg.yaw << ", ";
            out << "\"dist\": " << arg.dist << ", ";
            out << "\"fov\": " << arg.fov;
          } else if constexpr (std::is_same_v<T, EventSetLabel>) {
            out << "\"set_label\", \"id\": " << arg.id << ", \"label\": \""
                << arg.label << "\"";
          } else if constexpr (std::is_same_v<T, EventSetClass>) {
            out << "\"set_class\", \"id\": " << arg.id << ", \"class\": \""
                << arg.className << "\"";
          } else if constexpr (std::is_same_v<T, EventSetLayer>) {
            out << "\"set_layer\", \"id\": " << arg.id << ", \"layer\": \""
                << arg.layer << "\"";
          }
          out << " }";
        },
        event);

    if (i < m_events.size() - 1)
      out << ",";
    out << "\n";
  }

  out << "]\n";
  out.close();
  std::cout << "Trace dumped to " << filename << std::endl;
}

} // namespace iii
