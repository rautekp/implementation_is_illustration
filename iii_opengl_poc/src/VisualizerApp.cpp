// Simple JSON parser aid
// We assume line-based JSON as dumped by Recorder
// [
//   { ... },
//   { ... }
// ]

#include "VisualizerApp.hpp"
#include <fstream>
#include <imgui.h>
#include <iostream>

VisualizerApp::VisualizerApp() {
  // Default Camera
  m_camera.dist = 5.0f;
  m_camera.pitch = 0.5f;
  m_camera.yaw = 0.5f;
  m_camera.fov = 45.0f;

  m_traceCamera = m_camera;
  m_userCamera = m_camera;
}

void VisualizerApp::loadTrace(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    return;
  }

  // Placeholder for file loading implementation
  // Future: Use nlohmann/json or regex parser
  std::cout << "File loading not yet implemented for custom traces."
            << std::endl;
}

// ... Wait, implementing a parser is too much churn.
// Alternative: Modify main.cpp to *generate* the trace in memory by running the
// example code *inside* it, but just storing events instead of rendering. Then
// Visualizer plays that valid vector. This achieves "Visualizer UI" goal
// without file I/O complexity for now. We can simply link `example_matrix`
// logic or copy it into `VisualizerApp::generateDemoTrace()`.

void VisualizerApp::update(float dt) {
  ImGuiIO &io = ImGui::GetIO();

  // --- Input Handling ---
  if (!io.WantCaptureMouse) {
    // Orbit (Right Mouse)
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      std::cout << "DEBUG: Right Mouse Down" << std::endl;
      m_isManualInteraction = true;
      m_transitionAlpha = 0.0f; // Switch to manual

      m_userCamera.yaw -= io.MouseDelta.x * 0.01f;
      m_userCamera.pitch -= io.MouseDelta.y * 0.01f;

      // Clamp pitch to avoid gimbal lock/flipping
      if (m_userCamera.pitch < -1.5f)
        m_userCamera.pitch = -1.5f;
      if (m_userCamera.pitch > 1.5f)
        m_userCamera.pitch = 1.5f;
    }

    // Zoom (Scroll)
    if (io.MouseWheel != 0.0f) {
      m_isManualInteraction = true;
      m_transitionAlpha = 0.0f;
      m_userCamera.dist -= io.MouseWheel * 0.5f;
      if (m_userCamera.dist < 1.0f)
        m_userCamera.dist = 1.0f;
    }
  } else {
    // If user interacts with UI, we don't necessarily switch to manual cam,
    // unless they were already dragging?
    // Let's keep it simple.
  }

  if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
    m_isManualInteraction = false;
  }

  // --- Trace Camera Logic ---
  // If playing, we want to gently transition back to trace camera
  if (m_playing) {
    // Increase alpha to blend back
    m_transitionAlpha += dt * 2.0f; // 0.5s transition
    if (m_transitionAlpha > 1.0f)
      m_transitionAlpha = 1.0f;
  }
  // If paused and NOT interacting, we stay where we are (User Cam or blended).

  // Lerp Camera
  // Lerp(User, Trace, Alpha)
  auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

  m_camera.dist =
      lerp(m_userCamera.dist, m_traceCamera.dist, m_transitionAlpha);
  m_camera.pitch =
      lerp(m_userCamera.pitch, m_traceCamera.pitch, m_transitionAlpha);

  // Yaw lerp is tricky due to wrapping, but for orbital it's fine usually
  // unless > 360 diff. Shortest path interpolation would be better, but simple
  // linear for now.
  m_camera.yaw = lerp(m_userCamera.yaw, m_traceCamera.yaw, m_transitionAlpha);
  m_camera.fov = lerp(m_userCamera.fov, m_traceCamera.fov, m_transitionAlpha);

  // --- Playback ---
  if (m_playing) {
    m_timeAccumulator += dt * m_playbackSpeed;
    if (m_timeAccumulator > 1.0f) {
      // Safety cap handled by caller in main.cpp
    }
  }

  // Sync user cam to current to avoid jumps when pausing?
  // If we are fully tracking trace (alpha=1), update user cam too so if we
  // pause, we start from here.
  if (m_transitionAlpha >= 0.99f) {
    m_userCamera = m_traceCamera;
  }
}

void VisualizerApp::updateCameraProj(float aspect) {
  // Calc Camera Pos
  float cx = std::cos(m_camera.yaw) * std::cos(m_camera.pitch) * m_camera.dist;
  float cy = std::sin(m_camera.pitch) * m_camera.dist;
  float cz = std::sin(m_camera.yaw) * std::cos(m_camera.pitch) * m_camera.dist;

  MathHelpers::perspective(m_camera.fov * (float)M_PI / 180.0f, aspect, 0.1f,
                           100.0f, m_projMat);
  MathHelpers::lookAt(cx, cy, cz, 0, 0, 0, 0, 1, 0,
                      m_viewMat); // Look at origin
}

bool VisualizerApp::project(float x, float y, float z, float &sx, float &sy) {
  float v[4] = {x, y, z, 1.0f};
  float v_view[4];
  MathHelpers::mult(m_viewMat, v, v_view);
  float v_clip[4];
  MathHelpers::mult(m_projMat, v_view, v_clip);

  if (v_clip[3] == 0)
    return false;
  float invW = 1.0f / v_clip[3];

  float ndc_x = v_clip[0] * invW;
  float ndc_y = v_clip[1] * invW;
  float ndc_z = v_clip[2] * invW;

  // Check if behind camera
  if (v_clip[3] < 0)
    return false;

  // Map to window
  // ImGui coordinates: (0,0) is top-left
  // NDC (-1, -1) bottom-left -> ImGui
  // ndc_y needs flip?
  // ImGui y goes down. NDC y goes up.

  sx = (ndc_x * 0.5f + 0.5f) * m_viewport[2] + m_viewport[0];
  sy = (1.0f - (ndc_y * 0.5f + 0.5f)) * m_viewport[3] + m_viewport[1];

  return true;
}

#include "TestCases.hpp"
#include <iii/Parameter.hpp>
#include <regex>

void VisualizerApp::generateDemo(const std::string &name) {
  m_currentDemoName = name;
  if (name == "Matrix Demo") {
    m_trace = TestCases::generate_matrix_demo();
  } else if (name == "Solar System") {
    m_trace = TestCases::generate_solar_system();
  }
  // Reset state
  m_currentStep = 0;
  m_timeAccumulator = 0.0f;
  m_playing = false;
  m_renderer->reset(); // Clear old objects

  // Set initial camera from trace if available?
  // We scan for first camera event
  for (const auto &e : m_trace) {
    if (std::holds_alternative<iii::EventSetCamera>(e)) {
      const auto &cam = std::get<iii::EventSetCamera>(e);
      m_traceCamera.dist = cam.dist;
      m_traceCamera.pitch = cam.pitch;
      m_traceCamera.yaw = cam.yaw;
      m_traceCamera.fov = cam.fov;
      break;
    }
  }
  // Sync user camera
  m_userCamera = m_traceCamera;
}

void VisualizerApp::parseFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Error opening file: " << filename << std::endl;
    return;
  }

  m_trace.clear();
  std::string line;
  while (std::getline(file, line)) {
    // TODO: Implement parsing.
  }

  std::cout << "Loaded " << m_trace.size() << " events (Mock)." << std::endl;
}

void VisualizerApp::renderUI() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Load Trace...")) {
        std::cout << "Open File Dialog not implemented natively." << std::endl;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Demos")) {
      if (ImGui::MenuItem("Matrix Demo")) {
        generateDemo("Matrix Demo");
      }
      if (ImGui::MenuItem("Solar System")) {
        generateDemo("Solar System");
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  ImGui::Begin("Timeline");

  if (ImGui::Button(m_playing ? "Pause" : "Play")) {
    m_playing = !m_playing;
  }
  ImGui::SameLine();
  ImGui::SliderInt("Step", &m_currentStep, 0, (int)m_trace.size(), "Step %d");

  ImGui::Text("Message: %s", m_currentMessage.c_str());
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1, 1, 0, 1), "Code:");
  ImGui::TextUnformatted(m_currentCode.c_str());

  ImGui::End();

  // Legend Window
  ImGui::Begin("Legend");
  if (m_renderer) {
    for (const auto &obj : m_renderer->getObjects()) {
      if (!obj.visible)
        continue;
      if (obj.semantic == "Vector")
        continue; // Skip vectors in legend for now

      std::string name = obj.label.empty() ? std::to_string(obj.id) : obj.label;
      ImGui::BeginGroup();
      ImGui::ColorButton(
          (name + "##btn").c_str(), ImVec4(obj.r, obj.g, obj.b, 1.0f),
          ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
          ImVec2(10, 10));
      ImGui::SameLine();
      ImGui::Text("%s", name.c_str());
      ImGui::EndGroup();
    }
  }
  ImGui::End();

  // Parameters Panel
  ImGui::SetNextWindowPos(ImVec2(10, 260),
                          ImGuiCond_FirstUseEver); // Below Legend
  ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
    auto &params = iii::ParameterRegistry::getDoubles();
    bool changed = false;
    if (params.empty()) {
      ImGui::TextDisabled("No parameters registered.");
    }
    for (auto &[name, val] : params) {
      float fVal = (float)val;
      if (ImGui::DragFloat(name.c_str(), &fVal, 0.001f, 0.0001f, 10.0f)) {
        val = (double)fVal;
        changed = true;
      }
    }
    if (changed) {
      if (!m_currentDemoName.empty()) {
        generateDemo(m_currentDemoName);
      }
    }
  }
  ImGui::End();

  // 3D Labels Overlay
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::Begin("Overlay", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                   ImGuiWindowFlags_NoBackground |
                   ImGuiWindowFlags_NoDecoration |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoFocusOnAppearing |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  if (m_renderer) {
    for (const auto &obj : m_renderer->getObjects()) {
      if (!obj.visible)
        continue;
      if (obj.label.empty())
        continue;

      float sx, sy;
      float ox = obj.x;
      float oy = obj.y;
      float oz = obj.z;

      if (obj.semantic == "Frame") {
        ox = obj.matrix[12];
        oy = obj.matrix[13];
        oz = obj.matrix[14];
      }

      if (project(ox, oy, oz, sx, sy)) {
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(sx, sy), ImColor(1.0f, 1.0f, 1.0f), obj.label.c_str());
      }
    }
  }
  ImGui::End();
}
