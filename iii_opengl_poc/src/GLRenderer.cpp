#include "GLRenderer.hpp"
#include <GLFW/glfw3.h>
#include <iii/Math.hpp>
#include <iii/Vector.hpp>
#include <cmath>
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
        } else if constexpr (std::is_same_v<T, iii::EventSetClass>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.className = arg.className;
              break;
            }
          }
        } else if constexpr (std::is_same_v<T, iii::EventSetLayer>) {
          for (auto &obj : m_objects) {
            if (obj.id == arg.id) {
              obj.layer = arg.layer;
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

bool GLRenderer::isLayerVisible(const RenderObject &obj,
                                const std::map<std::string, bool> &layerVis) const {
  if (obj.layer.empty()) return true;
  auto it = layerVis.find(obj.layer);
  if (it == layerVis.end()) return true;
  return it->second;
}

// ============================================================
// Geometry helpers
// ============================================================

void GLRenderer::buildFrame(float dx, float dy, float dz,
                            float &ux, float &uy, float &uz,
                            float &vx, float &vy, float &vz) {
  // Find a vector not parallel to (dx,dy,dz)
  float ax = 0, ay = 1, az = 0;
  if (fabsf(dy) > 0.9f) { ax = 1; ay = 0; az = 0; }
  // u = normalize(cross(d, a))
  ux = dy * az - dz * ay;
  uy = dz * ax - dx * az;
  uz = dx * ay - dy * ax;
  float ulen = sqrtf(ux*ux + uy*uy + uz*uz);
  if (ulen < 1e-8f) { ux = 1; uy = 0; uz = 0; return; }
  ux /= ulen; uy /= ulen; uz /= ulen;
  // v = cross(d, u)
  vx = dy * uz - dz * uy;
  vy = dz * ux - dx * uz;
  vz = dx * uy - dy * ux;
}

void GLRenderer::drawSphere(float cx, float cy, float cz, float radius,
                            int seg, float cr, float cg, float cb, bool wireframe) {
  const float PI = 3.14159265f;
  GLenum mode = wireframe ? GL_LINE_STRIP : GL_TRIANGLE_STRIP;
  for (int lat = 0; lat < seg; ++lat) {
    float t1 = PI * lat / seg, t2 = PI * (lat + 1) / seg;
    glBegin(mode);
    glColor3f(cr, cg, cb);
    for (int lon = 0; lon <= seg; ++lon) {
      float p = 2.0f * PI * lon / seg;
      float cp2 = cosf(p), sp = sinf(p);
      float nx1 = sinf(t1)*cp2, ny1 = cosf(t1), nz1 = sinf(t1)*sp;
      float nx2 = sinf(t2)*cp2, ny2 = cosf(t2), nz2 = sinf(t2)*sp;
      glNormal3f(nx1, ny1, nz1);
      glVertex3f(cx + radius*nx1, cy + radius*ny1, cz + radius*nz1);
      glNormal3f(nx2, ny2, nz2);
      glVertex3f(cx + radius*nx2, cy + radius*ny2, cz + radius*nz2);
    }
    glEnd();
  }
}

void GLRenderer::drawCylinder(float x1, float y1, float z1,
                              float x2, float y2, float z2,
                              float radius, int seg,
                              float cr, float cg, float cb, bool wireframe) {
  float dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
  float len = sqrtf(dx*dx + dy*dy + dz*dz);
  if (len < 1e-8f) return;
  dx /= len; dy /= len; dz /= len;

  float ux, uy, uz, vx, vy, vz;
  buildFrame(dx, dy, dz, ux, uy, uz, vx, vy, vz);

  const float PI = 3.14159265f;
  GLenum mode = wireframe ? GL_LINE_STRIP : GL_TRIANGLE_STRIP;
  glBegin(mode);
  glColor3f(cr, cg, cb);
  for (int i = 0; i <= seg; ++i) {
    float angle = 2.0f * PI * i / seg;
    float ca = cosf(angle), sa = sinf(angle);
    float rnx = ux * ca + vx * sa;
    float rny = uy * ca + vy * sa;
    float rnz = uz * ca + vz * sa;
    glNormal3f(rnx, rny, rnz);
    glVertex3f(x1 + radius * rnx, y1 + radius * rny, z1 + radius * rnz);
    glVertex3f(x2 + radius * rnx, y2 + radius * rny, z2 + radius * rnz);
  }
  glEnd();
}

void GLRenderer::drawCone(float bx, float by, float bz,
                          float tx, float ty, float tz,
                          float radius, int seg,
                          float cr, float cg, float cb, bool wireframe) {
  float dx = tx - bx, dy = ty - by, dz = tz - bz;
  float len = sqrtf(dx*dx + dy*dy + dz*dz);
  if (len < 1e-8f) return;
  dx /= len; dy /= len; dz /= len;

  float ux, uy, uz, vx, vy, vz;
  buildFrame(dx, dy, dz, ux, uy, uz, vx, vy, vz);

  const float PI = 3.14159265f;
  GLenum mode = wireframe ? GL_LINE_STRIP : GL_TRIANGLE_FAN;
  // Cone surface normal: slope ratio accounts for radius/length
  float slopeN = radius / len; // how much normal tilts along axis
  glBegin(mode);
  glColor3f(cr, cg, cb);
  glNormal3f(dx, dy, dz); // tip normal approximation
  glVertex3f(tx, ty, tz); // tip
  for (int i = 0; i <= seg; ++i) {
    float angle = 2.0f * PI * i / seg;
    float ca = cosf(angle), sa = sinf(angle);
    float rnx = ux*ca + vx*sa;
    float rny = uy*ca + vy*sa;
    float rnz = uz*ca + vz*sa;
    // Normal: radial + axis slope
    float snx = rnx + dx * slopeN;
    float sny = rny + dy * slopeN;
    float snz = rnz + dz * slopeN;
    float snl = sqrtf(snx*snx+sny*sny+snz*snz);
    if (snl > 1e-8f) { snx/=snl; sny/=snl; snz/=snl; }
    glNormal3f(snx, sny, snz);
    glVertex3f(bx + radius*(ux*ca + vx*sa),
               by + radius*(uy*ca + vy*sa),
               bz + radius*(uz*ca + vz*sa));
  }
  glEnd();
}

void GLRenderer::drawDisc(float cx, float cy, float cz,
                          float nx, float ny, float nz,
                          float radius, int seg,
                          float cr, float cg, float cb) {
  float len = sqrtf(nx*nx + ny*ny + nz*nz);
  if (len < 1e-8f) return;
  nx /= len; ny /= len; nz /= len;

  float ux, uy, uz, vx, vy, vz;
  buildFrame(nx, ny, nz, ux, uy, uz, vx, vy, vz);

  const float PI = 3.14159265f;
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(cr, cg, cb);
  glNormal3f(nx, ny, nz); // all disc vertices share the face normal
  glVertex3f(cx, cy, cz);
  for (int i = 0; i <= seg; ++i) {
    float angle = 2.0f * PI * i / seg;
    float ca = cosf(angle), sa = sinf(angle);
    glVertex3f(cx + radius*(ux*ca + vx*sa),
               cy + radius*(uy*ca + vy*sa),
               cz + radius*(uz*ca + vz*sa));
  }
  glEnd();
}

// ============================================================
// Layer-only render (no class styles)
// ============================================================
void GLRenderer::render(const std::map<std::string, bool> &layerVis) {
  std::map<std::string, ClassStyle> empty;
  render(layerVis, empty);
}

// ============================================================
// Full style-aware render
// ============================================================
void GLRenderer::render(const std::map<std::string, bool> &layerVis,
                        const std::map<std::string, ClassStyle> &classStyles) {
  glPointSize(10.0f);
  glLineWidth(2.0f);

  // Setup lighting for styled shapes
  GLfloat lightPos[] = {5.0f, 10.0f, 7.0f, 0.0f}; // directional
  GLfloat lightAmb[] = {0.25f, 0.25f, 0.25f, 1.0f};
  GLfloat lightDif[] = {0.75f, 0.75f, 0.75f, 1.0f};
  GLfloat lightSpc[] = {0.4f, 0.4f, 0.4f, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpc);
  GLfloat matSpc[] = {0.3f, 0.3f, 0.3f, 1.0f};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpc);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);

  auto getStyle = [&](const RenderObject &obj) -> const ClassStyle* {
    if (obj.className.empty()) return nullptr;
    auto it = classStyles.find(obj.className);
    if (it == classStyles.end()) return nullptr;
    if (it->second.shape == ClassStyle::Default) return nullptr;
    return &it->second;
  };

  auto objColor = [](const RenderObject &obj, const ClassStyle *style,
                     float &cr, float &cg, float &cb) {
    if (style && style->overrideColor) {
      cr = style->colorR; cg = style->colorG; cb = style->colorB;
    } else {
      cr = obj.r; cg = obj.g; cb = obj.b;
    }
  };

  // Helper: resolve endpoint coordinates for a Line or Vector
  auto resolveLineEndpoints = [&](const RenderObject &obj,
                                  float &x1, float &y1, float &z1,
                                  float &x2, float &y2, float &z2) -> bool {
    if (obj.semantic == "Vector") {
      x1 = 0; y1 = 0; z1 = 0;
      if (obj.origin_id != 0) {
        for (const auto &o : m_objects) {
          if (o.id == obj.origin_id) { x1 = o.x; y1 = o.y; z1 = o.z; break; }
        }
      }
      x2 = x1 + obj.x; y2 = y1 + obj.y; z2 = z1 + obj.z;
      return true;
    } else if (obj.semantic == "Line") {
      bool f1 = false, f2 = false;
      for (const auto &pt : m_objects) {
        if (pt.id == obj.start_id) { x1 = pt.x; y1 = pt.y; z1 = pt.z; f1 = true; }
        if (pt.id == obj.end_id) { x2 = pt.x; y2 = pt.y; z2 = pt.z; f2 = true; }
      }
      return f1 && f2;
    }
    return false;
  };

  // --- Pass 1: Default points ---
  glBegin(GL_POINTS);
  for (const auto &obj : m_objects) {
    if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
    if (obj.semantic != "Point") continue;
    if (getStyle(obj)) continue;
    glColor3f(obj.r, obj.g, obj.b);
    glVertex3f(obj.x, obj.y, obj.z);
  }
  glEnd();

  // --- Pass 2: Styled points (Sphere) — with lighting ---
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  for (const auto &obj : m_objects) {
    if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
    if (obj.semantic != "Point") continue;
    auto *s = getStyle(obj);
    if (!s) continue;
    float cr, cg, cb; objColor(obj, s, cr, cg, cb);
    if (s->shape == ClassStyle::Sphere)
      drawSphere(obj.x, obj.y, obj.z, s->radius, s->tessellation, cr, cg, cb, s->wireframe);
  }
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);

  // --- Pass 3: Default lines (GL_LINES) ---
  glBegin(GL_LINES);
  for (const auto &obj : m_objects) {
    if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
    if (obj.semantic != "Vector" && obj.semantic != "Line") continue;
    if (getStyle(obj)) continue; // styled separately
    float x1, y1, z1, x2, y2, z2;
    if (!resolveLineEndpoints(obj, x1, y1, z1, x2, y2, z2)) continue;
    glColor3f(obj.r, obj.g, obj.b);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
  }
  glEnd();

  // --- Pass 4: Styled lines/vectors (Cylinder, Arrow) — with lighting ---
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  for (const auto &obj : m_objects) {
    if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
    if (obj.semantic != "Vector" && obj.semantic != "Line") continue;
    auto *s = getStyle(obj);
    if (!s) continue;

    float x1, y1, z1, x2, y2, z2;
    if (!resolveLineEndpoints(obj, x1, y1, z1, x2, y2, z2)) continue;
    float cr, cg, cb; objColor(obj, s, cr, cg, cb);

    if (s->shape == ClassStyle::Cylinder) {
      drawCylinder(x1, y1, z1, x2, y2, z2,
                   s->cylinderRadius, s->tessellation, cr, cg, cb, s->wireframe);
      if (s->endcaps) {
        float dx = x2-x1, dy = y2-y1, dz = z2-z1;
        float len = sqrtf(dx*dx+dy*dy+dz*dz);
        if (len > 1e-8f) {
          dx/=len; dy/=len; dz/=len;
          drawDisc(x1,y1,z1, -dx,-dy,-dz, s->cylinderRadius, s->tessellation, cr,cg,cb);
          drawDisc(x2,y2,z2, dx,dy,dz, s->cylinderRadius, s->tessellation, cr,cg,cb);
        }
      }
    } else if (s->shape == ClassStyle::Arrow) {
      // Shorten body to make room for cone
      float dx = x2-x1, dy = y2-y1, dz = z2-z1;
      float len = sqrtf(dx*dx+dy*dy+dz*dz);
      if (len < 1e-8f) continue;
      float ndx = dx/len, ndy = dy/len, ndz = dz/len;
      float bodyLen = len - s->coneLength;
      if (bodyLen < 0) bodyLen = len * 0.5f;
      float bx2 = x1 + ndx*bodyLen, by2 = y1 + ndy*bodyLen, bz2 = z1 + ndz*bodyLen;

      // Cylinder body
      drawCylinder(x1,y1,z1, bx2,by2,bz2,
                   s->cylinderRadius, s->tessellation, cr,cg,cb, s->wireframe);
      // Cone head
      drawCone(bx2,by2,bz2, x2,y2,z2,
               s->coneRadius, s->tessellation, cr,cg,cb, s->wireframe);
      // Endcap at start
      if (s->endcaps) {
        drawDisc(x1,y1,z1, -ndx,-ndy,-ndz, s->cylinderRadius, s->tessellation, cr,cg,cb);
      }
      // Disc at cone base
      drawDisc(bx2,by2,bz2, -ndx,-ndy,-ndz, s->coneRadius, s->tessellation, cr,cg,cb);
    }
  }
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);

  // --- Pass 5: Frames ---
  glBegin(GL_LINES);
  for (const auto &obj : m_objects) {
    if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
    if (obj.semantic == "Frame") {
      double tx = obj.matrix[12], ty = obj.matrix[13], tz = obj.matrix[14];
      glColor3f(1,0,0);
      glVertex3f(tx,ty,tz);
      glVertex3f(tx+obj.matrix[0],ty+obj.matrix[1],tz+obj.matrix[2]);
      glColor3f(0,1,0);
      glVertex3f(tx,ty,tz);
      glVertex3f(tx+obj.matrix[4],ty+obj.matrix[5],tz+obj.matrix[6]);
      glColor3f(0,0,1);
      glVertex3f(tx,ty,tz);
      glVertex3f(tx+obj.matrix[8],ty+obj.matrix[9],tz+obj.matrix[10]);
    }
  }
  glEnd();
}

std::vector<std::string> GLRenderer::getLayers() const {
  std::vector<std::string> result;
  for (const auto &obj : m_objects) {
    if (!obj.layer.empty()) {
      bool found = false;
      for (const auto &l : result) { if (l == obj.layer) { found = true; break; } }
      if (!found) result.push_back(obj.layer);
    }
  }
  return result;
}

std::vector<std::string> GLRenderer::getClasses() const {
  std::vector<std::string> result;
  for (const auto &obj : m_objects) {
    if (!obj.className.empty()) {
      bool found = false;
      for (const auto &c : result) { if (c == obj.className) { found = true; break; } }
      if (!found) result.push_back(obj.className);
    }
  }
  return result;
}
