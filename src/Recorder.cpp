#include "iii/Recorder.hpp"
#include "iii/IEventListener.hpp"

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
