#pragma once

#include <Eigen/Dense>
#include <autodiff/forward/real.hpp>
#include <autodiff/forward/real/eigen.hpp>
#include <iostream>

// Always include Recorder, but it will only be used if Visualize=true
#include "iii/Recorder.hpp"

namespace iii {

// Helper struct to conditionally hold visual data
struct VisualData {
  size_t id = 0;
  std::string label;
  Color color = {0.5f, 0.5f, 0.5f};
  std::string semantic = "Point";
};
struct NoVisualData {};

template <typename T, bool Visualize = false>
class Vector3 : public Eigen::Matrix<T, 3, 1> {
public:
  using Base = Eigen::Matrix<T, 3, 1>;

  // Conditional Data Member
#ifdef __cpp_lib_type_trait_variable_templates
  using DataMember = std::conditional_t<Visualize, VisualData, NoVisualData>;
#else
  using DataMember =
      typename std::conditional<Visualize, VisualData, NoVisualData>::type;
#endif
  [[no_unique_address]] DataMember
      m_data; // attribute requires C++20, but compatible

  // 1. Constructor from coordinates
  Vector3(T x, T y, T z) : Base(x, y, z) { register_visual(); }

  // 2. Default Constructor
  Vector3() : Base() { register_visual(); }

  // 3. Constructor from Eigen base
  Vector3(const Base &other) : Base(other) { register_visual(); }

  // 4. Templated Constructor from Eigen expressions
  template <typename Derived>
  Vector3(const Eigen::MatrixBase<Derived> &other) : Base(other) {
    register_visual();
  }

  // 5. Copy Constructor
  Vector3(const Vector3 &other) : Base(other) {
    register_visual();
    if constexpr (Visualize) {
      if constexpr (std::is_same_v<decltype(other), const Vector3<T, true> &>) {
        m_data.semantic = other.m_data.semantic;
        m_data.label = other.m_data.label;
        m_data.color = other.m_data.color;
      }
      if (m_data.id != 0) {
        Recorder::get().record(EventSetSemantic{m_data.id, m_data.semantic});
        Recorder::get().record(EventSetColor{m_data.id, m_data.color});
      }
    }
  }

  // Support copying from DIFFERENT visualization mode (e.g. Visual from
  // NonVisual)
  template <bool OtherVis>
  Vector3(const Vector3<T, OtherVis> &other) : Base(other) {
    register_visual();
    // If we are visual, we just start fresh (default semantic/color)
    // If other was visual, maybe we copy properties?
    if constexpr (Visualize && OtherVis) {
      m_data.semantic = other.m_data.semantic;
      m_data.label = other.m_data.label;
      m_data.color = other.m_data.color;
      if (m_data.id != 0) {
        Recorder::get().record(EventSetSemantic{m_data.id, m_data.semantic});
        Recorder::get().record(EventSetColor{m_data.id, m_data.color});
      }
    }
  }

  // 6. Move Constructor
  Vector3(Vector3 &&other) noexcept : Base(std::move(other)) {
    if constexpr (Visualize) {
      m_data.id = other.m_data.id;
      m_data.label = std::move(other.m_data.label);
      m_data.semantic = std::move(other.m_data.semantic);
      m_data.color = other.m_data.color;
      other.m_data.id = 0; // Invalidate source
    }
  }

  // 7. Copy Assignment
  Vector3 &operator=(const Vector3 &other) {
    if (this != &other) {
      Base::operator=(other);
      record_move();
    }
    return *this;
  }

  template <bool OtherVis>
  Vector3 &operator=(const Vector3<T, OtherVis> &other) {
    Base::operator=(other);
    record_move();
    return *this;
  }

  // 8. Move Assignment
  Vector3 &operator=(Vector3 &&other) noexcept {
    if (this != &other) {
      Base::operator=(std::move(other));
      if constexpr (Visualize) {
        if (m_data.id != 0) {
          Recorder::get().record(EventDestroy{m_data.id});
        }
        m_data.id = other.m_data.id;
        m_data.label = std::move(other.m_data.label);
        m_data.semantic = std::move(other.m_data.semantic);
        m_data.color = other.m_data.color;
        other.m_data.id = 0;
      }
    }
    return *this;
  }

  // 9. Assignment from Base
  template <typename Derived>
  Vector3 &operator=(const Eigen::MatrixBase<Derived> &other) {
    Base::operator=(other);
    record_move();
    return *this;
  }

  void register_visual() {
    if constexpr (Visualize) {
      if (m_data.id == 0) {
        m_data.id = Recorder::get().nextId();
        Recorder::get().record(
            EventCreate{m_data.id, m_data.label, m_data.color});
        Recorder::get().record(EventSetSemantic{m_data.id, m_data.semantic});
        record_move();
      }
    }
  }

  void record_move() {
    if constexpr (Visualize) {
      Recorder::get().record(EventMove{m_data.id, (double)this->x(),
                                       (double)this->y(), (double)this->z()});
    }
  }

  // API Methods
  void set_label(std::string l) {
    if constexpr (Visualize)
      m_data.label = l;
  }
  void set_color(float r, float g, float b) {
    if constexpr (Visualize) {
      m_data.color = {r, g, b};
      if (m_data.id != 0)
        Recorder::get().record(EventSetColor{m_data.id, m_data.color});
    }
  }
  void set_semantic(std::string s) {
    if constexpr (Visualize) {
      m_data.semantic = s;
      if (m_data.id != 0)
        Recorder::get().record(EventSetSemantic{m_data.id, m_data.semantic});
    }
  }

  template <bool OtherVis> void set_origin(const Vector3<T, OtherVis> &p) {
    if constexpr (Visualize && OtherVis) {
      if (m_data.id != 0 && p.m_data.id != 0) {
        Recorder::get().record(EventSetOrigin{m_data.id, p.m_data.id});
      }
    }
  }

  void set_visible(bool v) {
    if constexpr (Visualize) {
      if (m_data.id != 0)
        Recorder::get().record(EventSetVisible{m_data.id, v});
    }
  }

  ~Vector3() {
    if constexpr (Visualize) {
      if (m_data.id != 0)
        Recorder::get().record(EventDestroy{m_data.id});
    }
  }

  // Friend operators for drop-in Eigen compatibility + recording

  // Friend operators for drop-in Eigen compatibility + recording

  // Scalar ops are simpler, they preserve the visualization mode of the vector
  friend Vector3 operator*(const Vector3 &lhs, double scalar)
    requires Visualize
  {
    Vector3 result = static_cast<const Base &>(lhs) * scalar;
    if constexpr (Visualize) {
#ifdef III_ENABLE_VISUALS
      Recorder::get().record(
          EventScalarOp{result.m_data.id, lhs.m_data.id, scalar, "*"});
#endif
    }
    return result;
  }

  friend Vector3 operator*(double scalar, const Vector3 &rhs)
    requires Visualize
  {
    Vector3 result = scalar * static_cast<const Base &>(rhs);
    if constexpr (Visualize) {
#ifdef III_ENABLE_VISUALS
      Recorder::get().record(
          EventScalarOp{result.m_data.id, rhs.m_data.id, scalar, "*"});
#endif
    }
    return result;
  }

  friend std::ostream &operator<<(std::ostream &os, const Vector3 &v) {
    os << "[" << v.x() << ", " << v.y() << ", " << v.z() << "]";
    return os;
  }

  // Grant friendship to other template instantiations to access m_data
  template <typename U, bool V> friend class Vector3;
};

template <typename T, bool VisL, bool VisR>
Vector3<T, VisL || VisR> operator+(const Vector3<T, VisL> &lhs,
                                   const Vector3<T, VisR> &rhs)
  requires(VisL || VisR)
{
  Vector3<T, VisL || VisR> result =
      static_cast<const Eigen::Matrix<T, 3, 1> &>(lhs) +
      static_cast<const Eigen::Matrix<T, 3, 1> &>(rhs);
  if constexpr (VisL || VisR) {
    size_t lhsId = 0, rhsId = 0;
    if constexpr (VisL)
      lhsId = lhs.m_data.id;
    if constexpr (VisR)
      rhsId = rhs.m_data.id;

#ifdef III_ENABLE_VISUALS
    Recorder::get().record(EventBinaryOp{result.m_data.id, lhsId, rhsId, "+"});
#endif
  }
  return result;
}

template <typename T, bool VisL, bool VisR>
Vector3<T, VisL || VisR> operator-(const Vector3<T, VisL> &lhs,
                                   const Vector3<T, VisR> &rhs)
  requires(VisL || VisR)
{
  Vector3<T, VisL || VisR> result =
      static_cast<const Eigen::Matrix<T, 3, 1> &>(lhs) -
      static_cast<const Eigen::Matrix<T, 3, 1> &>(rhs);
  if constexpr (VisL || VisR) {
    size_t lhsId = 0, rhsId = 0;
    if constexpr (VisL)
      lhsId = lhs.m_data.id;
    if constexpr (VisR)
      rhsId = rhs.m_data.id;

#ifdef III_ENABLE_VISUALS
    Recorder::get().record(EventBinaryOp{result.m_data.id, lhsId, rhsId, "-"});
#endif
  }
  return result;
}

} // namespace iii
