#pragma once

#include <iii/IEventListener.hpp>
#include <glad/glad.h>
#include <map>
#include <string>
#include <vector>

// ============================================================
// ClassStyle — Visualizer-side mapping from class name to visual representation
// ============================================================

struct ClassStyle {
  enum Shape {
    Default = 0,
    Sphere,
    Cylinder,
    Arrow,
  };

  Shape shape = Default;

  // Common
  float radius = 0.2f;
  int tessellation = 12;
  bool wireframe = false;
  bool overrideColor = false;
  float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;

  // Cylinder / Arrow specific
  float cylinderRadius = 0.05f;
  bool endcaps = true;

  // Arrow specific
  float coneRadius = 0.12f;
  float coneLength = 0.3f;

  static constexpr int shapeCount = 4;
};

// ============================================================
// RenderObject — Internal state of a visual object
// ============================================================

struct RenderObject {
  size_t id;
  float x, y, z;
  float r, g, b;
  std::string label = "";
  std::string semantic = "Point";
  std::string className;
  std::string layer;
  size_t origin_id = 0;
  size_t start_id = 0;
  size_t end_id = 0;
  bool visible = true;
  double matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
};

// ============================================================
// GPUMesh — VAO/VBO/EBO for a unit mesh
// ============================================================

struct GPUMesh {
  GLuint vao = 0, vbo = 0, ebo = 0;
  int indexCount = 0;
  void cleanup();
};

// ============================================================
// GLRenderer — Modern OpenGL 4.1 renderer
// ============================================================

class GLRenderer : public iii::IEventListener {
public:
  GLRenderer();
  ~GLRenderer();

  void initGL(); // Call once after GL context is ready
  void shutdownGL();

  void onEvent(const iii::Event &e) override;
  void render(const float *viewMat, const float *projMat,
              const float *cameraPos,
              const std::map<std::string, bool> &layerVisibility,
              const std::map<std::string, ClassStyle> &classStyles);
  void resetObjects(); // Clear scene objects (NOT GPU resources)
  const std::vector<RenderObject> &getObjects() const { return m_objects; }

  std::vector<std::string> getLayers() const;
  std::vector<std::string> getClasses() const;

private:
  std::vector<RenderObject> m_objects;

  // --- Shaders ---
  GLuint m_litProgram = 0;   // Blinn-Phong for 3D shapes
  GLuint m_flatProgram = 0;  // Flat color for points/lines/frames

  // Lit shader uniform locations
  GLint m_litLocMVP = -1, m_litLocModel = -1, m_litLocNormalMat = -1;
  GLint m_litLocColor = -1, m_litLocLightDir = -1, m_litLocViewPos = -1;

  // Flat shader uniform locations
  GLint m_flatLocMVP = -1, m_flatLocColor = -1;

  // --- Dynamic buffers for points/lines ---
  GLuint m_dynVAO = 0, m_dynVBO = 0;

  // --- Mesh cache (keyed by tessellation) ---
  std::map<int, GPUMesh> m_sphereCache;
  std::map<int, GPUMesh> m_cylinderCache;
  std::map<int, GPUMesh> m_coneCache;
  std::map<int, GPUMesh> m_discCache;

  GPUMesh &getOrCreateSphere(int tess);
  GPUMesh &getOrCreateCylinder(int tess);
  GPUMesh &getOrCreateCone(int tess);
  GPUMesh &getOrCreateDisc(int tess);

  // --- Helpers ---
  bool isLayerVisible(const RenderObject &obj,
                      const std::map<std::string, bool> &layerVis) const;

  static GLuint compileShader(GLenum type, const char *src);
  static GLuint linkProgram(GLuint vert, GLuint frag);

  // Build a 4x4 model matrix that transforms a unit cylinder/cone
  // (along +Y, radius 1) to go from p1 to p2 with given radius
  static void buildSegmentMatrix(float x1, float y1, float z1,
                                 float x2, float y2, float z2,
                                 float radius, float mat[16]);

  // 4x4 matrix multiply: out = a * b (column-major)
  static void mat4Mul(const float *a, const float *b, float *out);
  // 3x3 normal matrix from model matrix (inverse transpose of upper-left 3x3)
  static void normalMat3(const float *model4x4, float *out3x3);
};
