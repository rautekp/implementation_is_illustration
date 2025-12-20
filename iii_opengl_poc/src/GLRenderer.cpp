#include "GLRenderer.hpp"
#include <GLFW/glfw3.h>
#include <iii/Math.hpp>
#include <iii/Vector.hpp>
#include <iostream>
#include <type_traits>
#include <variant>

// #include <iostream> // Keeping include for debug

void GLRenderer::onEvent(const iii::Event &e) {
  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, iii::EventCreate>) {
          m_objects.push_back({arg.id, 0.0f, 0.0f, 0.0f, arg.color.r,
                               arg.color.g, arg.color.b});
        } else if constexpr (std::is_same_v<T, iii::EventMove>) {
          // Find object by ID and update position
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.x = (float)arg.x;
              obj.y = (float)arg.y;
              obj.z = (float)arg.z;
              break;
            }
          }
        } else if constexpr (std::is_same_v<T, iii::EventDestroy>) {
          // Remove object
          std::erase_if(m_objects,
                        [&](const auto &obj) { return obj.id == arg.id; });
        } else if constexpr (std::is_same_v<T, iii::EventSetSemantic>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.semantic = arg.semantic;
              break;
            }
          }
        } else if constexpr (std::is_same_v<T, iii::EventSetOrigin>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.origin_id = arg.origin_id;
              break;
            }
          }
        } else if constexpr (std::is_same_v<T, iii::EventSetVisible>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.visible = arg.visible;
              break;
            }
          }
        } else if constexpr (std::is_same_v<T, iii::EventSetColor>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.r = arg.color.r;
              obj.g = arg.color.g;
              obj.b = arg.color.b;
              break;
            }
          }
        }
      },
      e);
}

void GLRenderer::render() {
  glPointSize(10.0f);
  glLineWidth(2.0f);

  // Draw Points
  glBegin(GL_POINTS);
  for (const auto &obj : m_objects) {
    if (!obj.visible)
      continue;
    if (obj.semantic == "Vector")
      continue;
    glColor3f(obj.r, obj.g, obj.b);
    glVertex3f(obj.x, obj.y, obj.z);
  }
  glEnd();

  // Draw Vectors (Lines from Origin)
  glBegin(GL_LINES);
  for (const auto &obj : m_objects) {
    if (!obj.visible)
      continue;
    if (obj.semantic == "Vector") {
      float startX = 0.0f, startY = 0.0f, startZ = 0.0f;

      // If attached to an origin, find it
      if (obj.origin_id != 0) {
        for (const auto &origin : m_objects) {
          if (origin.id == obj.origin_id) {
            startX = origin.x;
            startY = origin.y;
            startZ = origin.z;
            break;
          }
        }
      }

      glColor3f(obj.r, obj.g, obj.b);
      glVertex3f(startX, startY, startZ);
      glVertex3f(startX + obj.x, startY + obj.y, startZ + obj.z);
    }
  }
  glEnd();
}
