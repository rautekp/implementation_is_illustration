#pragma once

#include <iii/IEventListener.hpp>
#include <string>
#include <vector>

struct RenderObject {
  size_t id;
  float x, y, z;
  float r, g, b;
  std::string semantic = "Point";
  size_t origin_id = 0;
  bool visible = true;
};

class GLRenderer : public iii::IEventListener {
public:
  void onEvent(const iii::Event &e) override;
  void render();

private:
  std::vector<RenderObject> m_objects;
};
