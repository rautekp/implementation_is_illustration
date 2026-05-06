#include "GLRenderer.hpp"
#include "VisualizerApp.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>
#include <iostream>
#include <vector>

int main(int, char **) {
  if (!glfwInit())
    return 1;

  // Request OpenGL 4.1 Core Profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "III Visualizer", NULL, NULL);
  if (!window) {
    std::cerr << "Failed to create GLFW window (GL 4.1 required)\n";
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return 1;
  }
  std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

  // Setup ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 410");

  // App Instance
  VisualizerApp app;
  GLRenderer renderer;
  renderer.initGL();
  app.m_renderer = &renderer;

  // Generate Initial Demo
  app.generateDemo("Matrix Demo");
  std::cout << "Trace initialized.\n";

  app.m_playing = false;
  double lastTime = glfwGetTime();

  // Enable depth test and point size
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    double currentTime = glfwGetTime();
    float dt = (float)(currentTime - lastTime);
    lastTime = currentTime;

    // Start ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    app.update(dt);

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    app.m_viewport[0] = 0;
    app.m_viewport[1] = 0;
    app.m_viewport[2] = display_w;
    app.m_viewport[3] = display_h;

    float aspect = (float)display_w / (float)display_h;
    app.updateCameraProj(aspect);

    // Playback
    if (app.m_playing && app.m_currentStep < app.m_maxStep) {
      app.m_timeAccumulator += dt * app.m_playbackSpeed;
      const float stepInterval = 0.016f;
      while (app.m_timeAccumulator > stepInterval) {
        app.m_timeAccumulator -= stepInterval;
        if (app.m_currentStep < app.m_maxStep)
          app.m_currentStep++;
      }
    }

    // Process messages and camera events
    app.m_currentMessage = "";
    app.m_currentCode = "";
    if (!app.m_experiments.empty()) {
      auto &trace = app.m_experiments[app.m_activeTabIdx].trace;
      int limit = std::min(app.m_currentStep, (int)trace.size());
      for (int i = limit - 1; i >= 0; --i) {
        bool foundMsg = !app.m_currentMessage.empty();
        if (!foundMsg &&
            std::holds_alternative<iii::EventMessage>(trace[i])) {
          const auto &e = std::get<iii::EventMessage>(trace[i]);
          app.m_currentMessage = e.message;
          app.m_currentCode = e.code;
          foundMsg = true;
        }
        if (std::holds_alternative<iii::EventSetCamera>(trace[i])) {
          const auto &cam = std::get<iii::EventSetCamera>(trace[i]);
          app.m_traceCamera.dist = cam.dist;
          app.m_traceCamera.pitch = cam.pitch;
          app.m_traceCamera.yaw = cam.yaw;
          app.m_traceCamera.fov = cam.fov;
          break;
        }
      }
    }

    // Render UI
    app.renderUI();
    ImGui::Render();

    // GL rendering
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Feed Renderer with scene events
    renderer.resetObjects();
    for (auto &exp : app.m_experiments) {
      int lim = std::min(app.m_currentStep, (int)exp.trace.size());
      for (int i = 0; i < lim; ++i) {
        renderer.onEvent(exp.trace[i]);
      }
    }

    // Compute camera position for specular lighting
    float camPos[3];
    camPos[0] = app.m_camera.dist * cosf(app.m_camera.pitch) * sinf(app.m_camera.yaw);
    camPos[1] = app.m_camera.dist * sinf(app.m_camera.pitch);
    camPos[2] = app.m_camera.dist * cosf(app.m_camera.pitch) * cosf(app.m_camera.yaw);

    // Draw Scene (pass matrices as uniforms)
    renderer.render(app.m_viewMat, app.m_projMat, camPos,
                    app.m_layerVisibility, app.m_classStyles);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  renderer.shutdownGL();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
