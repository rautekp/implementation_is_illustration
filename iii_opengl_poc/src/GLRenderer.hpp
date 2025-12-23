#pragma once

#include <iii/IEventListener.hpp>
#include <string>
#include <vector>

struct RenderObject {
  size_t id;
  float x, y, z;
  float r, g, b;
  std::string label = "";
  std::string semantic = "Point";
  size_t origin_id = 0;
  bool visible = true;
  double matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
};

class GLRenderer : public iii::IEventListener {
public:
  void onEvent(const iii::Event &e) override;
  void render();
  void reset();
  const std::vector<RenderObject> &getObjects() const { return m_objects; }

private:
  std::vector<RenderObject> m_objects;
};
