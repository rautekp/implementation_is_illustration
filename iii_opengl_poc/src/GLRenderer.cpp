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
                               arg.color.g, arg.color.b, arg.label});
        } else if constexpr (std::is_same_v<T, iii::EventCreateRelation>) {
          RenderObject obj;
          obj.id = arg.id;
          obj.start_id = arg.start_id;
          obj.end_id = arg.end_id;
          obj.r = arg.color.r;
          obj.g = arg.color.g;
          obj.b = arg.color.b;
          obj.semantic = "Line"; // Visual semantic remains "Line"
          m_objects.push_back(obj);
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

        } else if constexpr (std::is_same_v<T, iii::EventSetMatrix>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              for (int i = 0; i < 16; ++i)
                obj.matrix[i] = arg.m[i];
              break;
            }
          }
        } else if constexpr (std::is_same_v<T, iii::EventSetLabel>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.label = arg.label;
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
    } else if (obj.semantic == "Line") {
      float x1 = 0, y1 = 0, z1 = 0;
      float x2 = 0, y2 = 0, z2 = 0;
      bool found1 = false, found2 = false;

      for (const auto &pt : m_objects) {
        if (pt.id == obj.start_id) {
          x1 = pt.x;
          y1 = pt.y;
          z1 = pt.z;
          found1 = true;
        }
        if (pt.id == obj.end_id) {
          x2 = pt.x;
          y2 = pt.y;
          z2 = pt.z;
          found2 = true;
        }
      }

      if (found1 && found2) {
        glColor3f(obj.r, obj.g, obj.b);
        glVertex3f(x1, y1, z1);
        glVertex3f(x2, y2, z2);
      }
    }
  }

  glEnd();

  // Draw Frames
  glBegin(GL_LINES);
  for (const auto &obj : m_objects) {
    if (!obj.visible)
      continue;
    if (obj.semantic == "Frame") {
      // Basis X (Red)
      double tx = obj.matrix[12];
      double ty = obj.matrix[13];
      double tz = obj.matrix[14];

      glColor3f(1.0f, 0.0f, 0.0f);
      glVertex3f(tx, ty, tz);
      glVertex3f(tx + obj.matrix[0], ty + obj.matrix[1], tz + obj.matrix[2]);

      // Basis Y (Green)
      glColor3f(0.0f, 1.0f, 0.0f);
      glVertex3f(tx, ty, tz);
      glVertex3f(tx + obj.matrix[4], ty + obj.matrix[5], tz + obj.matrix[6]);

      // Basis Z (Blue)
      glColor3f(0.0f, 0.0f, 1.0f);
      glVertex3f(tx, ty, tz);
      glVertex3f(tx + obj.matrix[8], ty + obj.matrix[9], tz + obj.matrix[10]);
    }
  }
  glEnd();
}

void GLRenderer::reset() { m_objects.clear(); }
