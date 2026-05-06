#pragma once
#include <glad/glad.h>
#include <vector>
#include <cmath>

struct MeshData {
  std::vector<float> vertices; // interleaved pos(3)+normal(3)
  std::vector<unsigned int> indices;
};

inline MeshData generateSphereMesh(int seg) {
  MeshData m;
  const float PI = 3.14159265f;
  for (int lat = 0; lat <= seg; ++lat) {
    float t = PI * lat / seg;
    float st = sinf(t), ct = cosf(t);
    for (int lon = 0; lon <= seg; ++lon) {
      float p = 2.0f * PI * lon / seg;
      float sp = sinf(p), cp = cosf(p);
      float nx = st * cp, ny = ct, nz = st * sp;
      m.vertices.insert(m.vertices.end(), {nx, ny, nz, nx, ny, nz});
    }
  }
  for (int lat = 0; lat < seg; ++lat) {
    for (int lon = 0; lon < seg; ++lon) {
      int a = lat * (seg + 1) + lon;
      int b = a + seg + 1;
      m.indices.insert(m.indices.end(), {(unsigned)a, (unsigned)b, (unsigned)(a+1),
                                          (unsigned)(a+1), (unsigned)b, (unsigned)(b+1)});
    }
  }
  return m;
}

inline MeshData generateCylinderMesh(int seg) {
  MeshData m;
  const float PI = 3.14159265f;
  // Two rings: y=0 (bottom), y=1 (top), radius=1
  for (int ring = 0; ring <= 1; ++ring) {
    float y = (float)ring;
    for (int i = 0; i <= seg; ++i) {
      float a = 2.0f * PI * i / seg;
      float nx = cosf(a), nz = sinf(a);
      m.vertices.insert(m.vertices.end(), {nx, y, nz, nx, 0.0f, nz});
    }
  }
  for (int i = 0; i < seg; ++i) {
    int a = i, b = i + seg + 1;
    m.indices.insert(m.indices.end(), {(unsigned)a, (unsigned)b, (unsigned)(a+1),
                                        (unsigned)(a+1), (unsigned)b, (unsigned)(b+1)});
  }
  return m;
}

inline MeshData generateConeMesh(int seg) {
  MeshData m;
  const float PI = 3.14159265f;
  // Base ring at y=0, radius=1; tip at y=1
  float slopeY = 1.0f / sqrtf(2.0f); // normal tilt
  float slopeR = 1.0f / sqrtf(2.0f);
  // Tip vertex (index 0)
  m.vertices.insert(m.vertices.end(), {0, 1, 0, 0, slopeY, 0});
  // Base ring
  for (int i = 0; i <= seg; ++i) {
    float a = 2.0f * PI * i / seg;
    float cx = cosf(a), cz = sinf(a);
    float nx = cx * slopeR, nz = cz * slopeR;
    m.vertices.insert(m.vertices.end(), {cx, 0, cz, nx, slopeY, nz});
  }
  for (int i = 0; i < seg; ++i) {
    m.indices.insert(m.indices.end(), {0u, (unsigned)(i+1), (unsigned)(i+2)});
  }
  return m;
}

inline MeshData generateDiscMesh(int seg) {
  MeshData m;
  const float PI = 3.14159265f;
  // Center (index 0)
  m.vertices.insert(m.vertices.end(), {0, 0, 0, 0, 1, 0});
  for (int i = 0; i <= seg; ++i) {
    float a = 2.0f * PI * i / seg;
    m.vertices.insert(m.vertices.end(), {cosf(a), 0, sinf(a), 0, 1, 0});
  }
  for (int i = 0; i < seg; ++i) {
    m.indices.insert(m.indices.end(), {0u, (unsigned)(i+1), (unsigned)(i+2)});
  }
  return m;
}

inline void uploadMesh(const MeshData &data, GLuint &vao, GLuint &vbo, GLuint &ebo, int &indexCount) {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(float), data.vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned), data.indices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);
  indexCount = (int)data.indices.size();
}
