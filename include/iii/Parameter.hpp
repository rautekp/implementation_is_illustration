#pragma once

#include <iostream>
#include <map>
#include <string>


namespace iii {

// Global Registry for Parameters (Only used when Visuals are enabled)
// Using inline static to be header-only friendly
struct ParameterRegistry {
  static std::map<std::string, double> &getDoubles() {
    static std::map<std::string, double> s_doubles;
    return s_doubles;
  }

  // Check if an override exists
  static bool hasOverride(const std::string &name) {
    return getDoubles().find(name) != getDoubles().end();
  }

  // Get override value
  static double getOverride(const std::string &name) {
    return getDoubles()[name];
  }

  // Set override value (called by Visualizer UI)
  static void setOverride(const std::string &name, double val) {
    getDoubles()[name] = val;
  }
};

template <typename T> class Parameter {
public:
  // Fast Path: Just a wrapper
  // Visual Path: Checks registry on construction

  Parameter(const std::string &name, T defaultValue) : m_value(defaultValue) {
#ifdef III_ENABLE_VISUALS
    // If the visualizer has set a value for this parameter, use it.
    // Otherwise, register it (or just implicitly let it be customizable next
    // frame)

    // Note: For the UI to know this parameter EXISTS, we might need to register
    // it if it's not in the map yet.
    auto &reg = ParameterRegistry::getDoubles();
    if (reg.find(name) == reg.end()) {
      reg[name] = defaultValue;
    } else {
      m_value = (T)reg[name];
    }
#endif
  }

  // Implicit conversion to T
  operator T() const { return m_value; }

  // Assignment
  Parameter &operator=(const T &val) {
    m_value = val;
    return *this;
  }

private:
  T m_value;
};

} // namespace iii
