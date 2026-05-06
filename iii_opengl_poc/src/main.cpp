#include "GLRenderer.hpp"
#include "VisualizerApp.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <iostream>
#include <vector>

int main(int, char **) {
  if (!glfwInit())
    return 1;
  // Window init
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "III Visualizer", NULL, NULL);
  if (!window)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;           // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL2_Init();

  // App Instance
  VisualizerApp app;
  GLRenderer renderer;
  app.m_renderer = &renderer;

  // Generate Initial Demo
  app.generateDemo("Matrix Demo"); // Starts with Matrix Demo by default
  std::cout << "Trace initialized.\n";

  // Init Playback
  app.m_playing = false;

  // Time tracking
  double lastTime = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    double currentTime = glfwGetTime();
    float dt = (float)(currentTime - lastTime);
    lastTime = currentTime;

    // Start ImGui Frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Update App Logic (Camera interpolation etc) - Moved here to have fresh
    // Input
    app.update(dt);

    // Viewport info for Projection
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    app.m_viewport[0] = 0;
    app.m_viewport[1] = 0;
    app.m_viewport[2] = display_w;
    app.m_viewport[3] = display_h;

    // Camera Update
    float aspect = (float)display_w / (float)display_h;
    app.updateCameraProj(aspect);

    // Logic (Playback step)
    if (app.m_playing && app.m_currentStep < app.m_trace.size()) {
      // Speed control
      app.m_timeAccumulator += dt * app.m_playbackSpeed;
      const float stepInterval = 0.016f; // ~60 fps
      while (app.m_timeAccumulator > stepInterval) {
        app.m_timeAccumulator -= stepInterval;
        if (app.m_currentStep < app.m_trace.size())
          app.m_currentStep++;
      }
    }

    // Process range events for Camera and Messages
    app.m_currentMessage = "";
    app.m_currentCode = "";
    // We only need the *latest* camera event in range [0, currentStep]
    for (int i = app.m_currentStep - 1; i >= 0; --i) {
      bool foundMsg = !app.m_currentMessage.empty();
      // Check message
      if (!foundMsg &&
          std::holds_alternative<iii::EventMessage>(app.m_trace[i])) {
        const auto &e = std::get<iii::EventMessage>(app.m_trace[i]);
        app.m_currentMessage = e.message;
        app.m_currentCode = e.code;
        foundMsg = true;
      }
      // Check camera
      if (std::holds_alternative<iii::EventSetCamera>(app.m_trace[i])) {
        const auto &cam = std::get<iii::EventSetCamera>(app.m_trace[i]);
        // Update target trace camera
        app.m_traceCamera.dist = cam.dist;
        app.m_traceCamera.pitch = cam.pitch;
        app.m_traceCamera.yaw = cam.yaw;
        app.m_traceCamera.fov = cam.fov;
        break;
      }
    }

    // Render UI (Controls + Labels)
    app.renderUI();

    // Rendering Scene
    ImGui::Render();

    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Depth too

    // Apply Camera
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(app.m_projMat);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(app.m_viewMat);

    // Feed Renderer
    renderer = GLRenderer(); // Reset state
    for (int i = 0; i < app.m_currentStep; ++i) {
      renderer.onEvent(app.m_trace[i]);
    }

    // Draw Scene
    renderer.render(app.m_layerVisibility, app.m_classStyles);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
