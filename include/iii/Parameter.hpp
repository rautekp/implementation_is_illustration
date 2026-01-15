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

  // Constructor for std::string (Legacy/General)
  Parameter(const std::string &name, T defaultValue) : m_value(defaultValue) {
    registerParameter(name, defaultValue);
  }

  // Optimized Constructor for string literals
  Parameter(const char *name, T defaultValue) : m_value(defaultValue) {
    registerParameter(name, defaultValue);
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

  // Helper to keep constructors clean and inline-able
  // We use a template to accept both string and char* without overhead in the
  // OFF case? Actually, standard overloading is fine.

  void registerParameter(const std::string &name, T defaultValue) {
#if defined(III_ENABLE_VISUALS) && !defined(III_NO_VISUALS_OVERRIDE)
    auto &reg = ParameterRegistry::getDoubles();
    if (reg.find(name) == reg.end()) {
      reg[name] = defaultValue;
    } else {
      m_value = (T)reg[name];
    }
#endif
  }

  // Overload for char* to avoid string construction if disabled
  void registerParameter(const char *name, T defaultValue) {
#if defined(III_ENABLE_VISUALS) && !defined(III_NO_VISUALS_OVERRIDE)
    // Only construct string if visuals are ENABLED
    registerParameter(std::string(name), defaultValue);
#endif
    // If visuals are disabled, this function is empty and 'name' is ignored.
    // No std::string created.
  }
};

} // namespace iii
