#include <iii/IEventListener.hpp>
#include <iii/Math.hpp>
#include <iii/Vector.hpp>
#include <iostream>


// A simple listener to verify real-time integration
struct ConsoleListener : public iii::IEventListener {
  void onEvent(const iii::Event &e) override {
    std::visit(
        [](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, iii::EventCreate>) {
            std::cout << "[Listener] Created Object ID:" << arg.id
                      << " Label:" << arg.label << "\n";
          } else if constexpr (std::is_same_v<T, iii::EventFunctionStart>) {
            std::cout << "[Listener] Call: " << arg.name << "\n";
          } else if constexpr (std::is_same_v<T, iii::EventFunctionEnd>) {
            std::cout << "[Listener] Return\n";
          }
        },
        e);
  }
};

// Define a type for convenience
using Vector3 = iii::Vector3<double>;

int main() {
  std::cout << "Starting Implementation is Illustration PoC..." << std::endl;

  ConsoleListener listener;
#ifdef III_ENABLE_VISUALS
  iii::Recorder::get().addListener(&listener);
#endif

  Vector3 a(0.0, 0.0, 0.0);
  a.set_label("Start");
  a.set_color(1.0f, 0.0f, 0.0f); // Red

  Vector3 b(10.0, 10.0, 10.0);
  b.set_label("End");
  b.set_color(0.0f, 1.0f, 0.0f); // Green

  // Perform linear interpolation
  // This should record: function_start(lerp) -> binary_ops... -> function_end
  Vector3 c = iii::lerp(a, b, 0.5);
  c.set_label("Midpoint");
  c.set_color(0.0f, 0.0f, 1.0f); // Blue

  std::cout << "Calculation complete. Result: " << c << std::endl;

#ifdef III_ENABLE_VISUALS
  iii::Recorder::get().dump("trace.json");
#endif

  return 0;
}
