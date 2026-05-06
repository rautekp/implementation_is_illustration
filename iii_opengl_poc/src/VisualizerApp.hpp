#pragma once

#include "GLRenderer.hpp"
#include <iii/Recorder.hpp>
#include <map>
#include <string>
#include <vector>

// Use the single-header json lib, or manual parsing?
// Plan said "Load trace.json".
// For simplicity without adding more manual deps, I'll use a simple manual
// parser or regex for the PoC OR since I wanted to be robust, I can assume
// nlohmann/json is available or just hack a parser. Actually, `command: git
// clone` meant I could have cloned json too. Let's stick to a very simple
// line-based parser or just regex for this PoC. Or effectively, just read the
// events line by line assuming the format I dumped (one event per line mostly).

class VisualizerApp {
public:
  VisualizerApp();
  void loadTrace(const std::string &filename);
  void update(float dt);
  void renderUI();

  // Playback
  int m_maxStep = 0;
  int m_currentStep = 0;
  bool m_playing = false;
  float m_playbackSpeed = 1.0f; // events per second? or just speed multiplier
  float m_timeAccumulator = 0.0f;
  std::string m_currentDemoName;

  // Trace Data
  // We need to store events. iii::Event is a variant.
  // However, we need to know *what* to display.
  // The GLRenderer stores the *current state*.
  // To support scrubbing, we either:
  struct ExperimentTab {
      std::string name;
      std::map<std::string, double> parameters;
      std::vector<iii::Event> trace;
  };

  std::vector<ExperimentTab> m_experiments;
  int m_activeTabIdx = 0;

  // We no longer have a single trace, but `m_currentStep` bounds all active traces.

  // Current Message/Code (extracted from the active tab or across tabs)
  std::string m_currentMessage;
  std::string m_currentCode;

  GLRenderer *m_renderer = nullptr; // Reference to renderer to reset/feed

  // Camera
  void updateCameraProj(float aspect);
  struct Camera {
    float fov = 45.0f;
    float dist = 5.0f;
    float pitch = 0.5f;
    float yaw = 0.5f;
    // Simple orbital camera
  };
  Camera m_camera;      // The actual camera used for rendering
  Camera m_traceCamera; // The target camera from the trace
  Camera m_userCamera;  // The user's manual override camera

  float m_transitionAlpha = 1.0f; // 0 (User/Manual) -> 1 (Trace/Anim)
  bool m_isManualInteraction = false;

  // Projection Helper
  // Returns true if visible (in front of camera)
  bool project(float x, float y, float z, float &sx, float &sy);

  // Cached Matrices for projection
  float m_viewMat[16];
  float m_projMat[16];
  int m_viewport[4];

  // Loading
  void generateDemo(const std::string &name);
  void regenerateTraces();
  void parseFile(const std::string &path);

  // GUI State
  bool m_showDemoWindow = true;
  bool m_autoCameraTracking = true;
  char m_inputFilePath[256] = "";
  std::map<std::string, bool> m_layerVisibility; // layer name -> visible
  std::map<std::string, ClassStyle> m_classStyles; // class name -> visual style

  void loadClassStyles(const std::string &demoName);
  void saveClassStyles(const std::string &demoName);
  std::string getStyleFilename(const std::string &demoName) const;

private:
  std::vector<iii::Event> parseTraceLine(const std::string &line);
};
