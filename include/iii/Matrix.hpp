#pragma once

#include "iii/Recorder.hpp"
#include "iii/Vector.hpp"
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>

namespace iii {

template <typename T, bool Visualize = false>
class Matrix4 : public Eigen::Matrix<T, 4, 4> {
public:
  using Base = Eigen::Matrix<T, 4, 4>;

  // Conditional Data Member (reusing VisualData structure pattern)
  // We can reuse VisualData or similar. Let's reuse struct VisualData from
  // Vector.hpp But wait, Vector.hpp is included. We need to access VisualData
  // definition. Ideally it should be moved to a common header. For now, let's
  // redefine similar struct or assuming we can use it if we make it
  // public/common. Actually, Vector.hpp defined it inside namespace iii.

  struct MatVisualData {
    size_t id = 0;
    std::string label;
    // Matrices (Frames) have basis colors usually, maybe just a main label
    // color?
  };
  struct NoVisualData {};

#ifdef __cpp_lib_type_trait_variable_templates
  using DataMember = std::conditional_t<Visualize, MatVisualData, NoVisualData>;
#else
  using DataMember =
      typename std::conditional<Visualize, MatVisualData, NoVisualData>::type;
#endif

  [[no_unique_address]] DataMember m_data;

  // Constructors
  Matrix4() : Base() { register_visual(); }
  Matrix4(const Base &other) : Base(other) { register_visual(); }

  template <typename Derived>
  Matrix4(const Eigen::MatrixBase<Derived> &other) : Base(other) {
    register_visual();
  }

  // Explicit Copy Constructor
  Matrix4(const Matrix4 &other) : Base(other) {
    if constexpr (Visualize) {
      m_data.label = other.m_data.label;
    }
    register_visual();
  }

  // Explicit Move Constructor
  Matrix4(Matrix4 &&other) noexcept : Base(std::move(other)) {
    if constexpr (Visualize) {
      m_data = other.m_data; // Transfer visual identity
      other.m_data.id = 0;   // Invalidate source
    }
    // No need to register, we stole one
  }

  // Custom constructor from identity, etc?
  // Eigen defaults.

  // Assignment operators to preserve ID

  // Move Assignment - preserve ID, update values
  Matrix4 &operator=(Matrix4 &&other) noexcept {
    if (this != &other) {
      Base::operator=(std::move(other));
      if constexpr (Visualize) {
        // Keep existing ID, do not overwrite m_data
        record_update();
      }
    }
    return *this;
  }

  Matrix4 &operator=(const Matrix4 &other) {
    if (this != &other) {
      Base::operator=(other);
      if constexpr (Visualize) {
        // Keep existing ID, do not overwrite m_data
        record_update();
      }
    }
    return *this;
  }

  template <typename OtherDerived>
  Matrix4 &operator=(const Eigen::MatrixBase<OtherDerived> &other) {
    Base::operator=(other);
    if constexpr (Visualize) {
      record_update();
    }
    return *this;
  }

  // Factories
  static Matrix4 Identity() { return Base::Identity(); }

  static Matrix4 Translate(T x, T y, T z) {
    Matrix4 m(Base::Identity());
    m(0, 3) = x;
    m(1, 3) = y;
    m(2, 3) = z;
    if constexpr (Visualize) {
      m.record_update();
    }
    return m;
  }

  static Matrix4 Rotate(T angle, const Vector3<T, Visualize> &axis) {
    Matrix4 m = Base::Identity();
    // Axis-Angle to rotation matrix
    // Eigen has AngleAxis
    Eigen::Matrix<T, 3, 3> rot =
        Eigen::AngleAxis<T>(angle, axis).toRotationMatrix();
    m.block(0, 0, 3, 3) = rot;

    if constexpr (Visualize)
      m.record_update();
    return m;
  }

  static Matrix4 Scale(T sx, T sy, T sz) {
    Matrix4 m = Base::Identity();
    m(0, 0) = sx;
    m(1, 1) = sy;
    m(2, 2) = sz;
    if constexpr (Visualize)
      m.record_update();
    return m;
  }

  // Multiply Matrix * Vector
  // We need to handle Vector3
  template <bool VecVis>
  Vector3<T, Visualize || VecVis> operator*(const Vector3<T, VecVis> &v) const {
    // Homogeneous multiplication
    Eigen::Matrix<T, 4, 1> v4;
    v4 << v.x(), v.y(), v.z(), 1.0;
    Eigen::Matrix<T, 4, 1> res4 = static_cast<const Base &>(*this) * v4;

    // Project back (w division? usually w=1 for affine)
    Vector3<T, Visualize || VecVis> result(res4.x(), res4.y(), res4.z());

    if constexpr (Visualize || VecVis) {
      size_t matId = 0;
      size_t vecId = 0;
      if constexpr (Visualize)
        matId = m_data.id;
      if constexpr (VecVis)
        vecId = v.m_data.id;

#ifdef III_ENABLE_VISUALS
      Recorder::get().record(EventTransform{result.m_data.id, matId, vecId});
#endif
    }
    return result;
  }

  // Multiply Matrix * Matrix
  template <bool OtherVis>
  Matrix4<T, Visualize || OtherVis>
  operator*(const Matrix4<T, OtherVis> &other) const {
    Matrix4<T, Visualize || OtherVis> result =
        static_cast<const Base &>(*this) * static_cast<const Base &>(other);
    // How to visualize matrix multiplication? Maybe just show result frame?
    // Logic: Composition of transforms.
    // For now, result just gets a new ID and initial value.
    return result;
  }

  void register_visual() {
    if constexpr (Visualize) {
      if (m_data.id == 0) {
        m_data.id = Recorder::get().nextId();
        // Label defaults?
        std::string l = m_data.label.empty() ? "Frame" : m_data.label;
        Recorder::get().record(EventCreate{m_data.id, l, {0.8, 0.8, 0.8}});
        Recorder::get().record(EventSetSemantic{m_data.id, "Frame"});
        record_update();
      }
    }
  }

  void record_update() {
    if constexpr (Visualize) {
      EventSetMatrix e;
      e.id = m_data.id;
      // Copy data. Eigen stores column-major by default.
      // We will dump it as is, and openGL user must know it's column major.
      const T *data = this->data();
      for (int i = 0; i < 16; ++i)
        e.m[i] = static_cast<double>(data[i]);
      Recorder::get().record(e);
    }
  }

  void set_label(std::string l) {
    if constexpr (Visualize)
      m_data.label = l;
    // Update label event? EventCreate already sent. Need EventSetLabel?
    // Not implemented in Recorder yet.
  }
};

} // namespace iii
