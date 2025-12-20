#pragma once

#include "iii/Function.hpp"
#include "iii/Recorder.hpp"
#include "iii/Vector.hpp"


namespace iii {

constexpr auto lerp = Function(
    "lerp", [](auto &&a, auto &&b, double t) { return a + (b - a) * t; });

} // namespace iii
