#pragma once

#include "iii/Recorder.hpp"
#include "iii/Vector.hpp"
#include <type_traits>
#include <utility>


namespace iii {

// Trait to detect if type is a visual Vector3
// We can expand this later for other visual types
template <typename T> struct is_visual_type : std::false_type {};

template <typename T>
struct is_visual_type<Vector3<T, true>> : std::true_type {};

template <typename... Args>
constexpr bool any_visual_v =
    (is_visual_type<std::decay_t<Args>>::value || ...);

template <typename Func> struct Function {
  const char *name;
  Func implementation;

  constexpr Function(const char *n, Func f)
      : name(n), implementation(std::move(f)) {}

  template <typename... Args> decltype(auto) operator()(Args &&...args) const {
    // 1. Check if we should record start
    bool recording = false;
    if constexpr (any_visual_v<Args...>) {
#ifdef III_ENABLE_VISUALS
      Recorder::get().function_start(name);
      recording = true;
#endif
    }

    // 2. Call implementation
    // We capture by value/move to return later, or just return directly if we
    // could simple call end before? No, we need result first if we want to
    // return it. But function_end doesn't take result (yet).
    // The issue: if we return decltype(auto), we can't easily put code AFTER
    // the return statement.
    // We can use a scope guard/destroyer.

    // Simple Scope Guard for recording end
    struct ScopeEnd {
      bool do_record;
      ~ScopeEnd() {
        if (do_record) {
#ifdef III_ENABLE_VISUALS
          Recorder::get().function_end();
#endif
        }
      }
    } ender{recording};

    return implementation(std::forward<Args>(args)...);
  }
};

// Valid C++17 deduction guide (though often implicit in C++20)
template <typename Func> Function(const char *, Func) -> Function<Func>;

} // namespace iii
