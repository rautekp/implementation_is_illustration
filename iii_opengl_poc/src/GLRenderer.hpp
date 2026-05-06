#pragma once

#include <iii/IEventListener.hpp>
#include <map>
#include <string>
#include <vector>

// ============================================================
// ClassStyle — Visualizer-side mapping from class name to visual representation
// The iii library knows nothing about these; they are purely renderer concepts.
// ============================================================

struct ClassStyle {
  enum Shape {
    Default = 0, // Use the object's semantic (Point, Vector, Line, Frame)
    Sphere,      // For points: solid/wireframe sphere
    Cylinder,    // For lines/vectors: tube between endpoints
    Arrow,       // For lines/vectors: cylinder body + cone arrowhead
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

  // UI helpers
  static constexpr int shapeCount = 4;
};

// ============================================================
// RenderObject
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
// GLRenderer
// ============================================================

class GLRenderer : public iii::IEventListener {
public:
  void onEvent(const iii::Event &e) override;
  void render();
  void render(const std::map<std::string, bool> &layerVisibility);
  void render(const std::map<std::string, bool> &layerVisibility,
              const std::map<std::string, ClassStyle> &classStyles);
  void reset();
  const std::vector<RenderObject> &getObjects() const { return m_objects; }

  std::vector<std::string> getLayers() const;
  std::vector<std::string> getClasses() const;

private:
  std::vector<RenderObject> m_objects;

  bool isLayerVisible(const RenderObject &obj,
                      const std::map<std::string, bool> &layerVis) const;

  // Shape drawing primitives
  void drawSphere(float cx, float cy, float cz, float radius,
                  int seg, float r, float g, float b, bool wireframe);
  void drawCylinder(float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float radius, int seg,
                    float r, float g, float b, bool wireframe);
  void drawCone(float baseX, float baseY, float baseZ,
                float tipX, float tipY, float tipZ,
                float radius, int seg,
                float r, float g, float b, bool wireframe);
  void drawDisc(float cx, float cy, float cz,
                float nx, float ny, float nz,
                float radius, int seg,
                float r, float g, float b);

  // Helper: build a perpendicular frame around an axis direction
  void buildFrame(float dx, float dy, float dz,
                  float &ux, float &uy, float &uz,
                  float &vx, float &vy, float &vz);
};
