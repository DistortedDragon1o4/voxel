#include <bitset>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

#include <assert.h>
#include <chrono>
#include <future>
#include <thread>

#include "chunkDataContainer.h"
#include "glm/trigonometric.hpp"
#include "stb/stb_image.h"

#include "../include/HUD.h"
#include "../include/camera.h"
#include "../include/chunkList.h"
#include "../include/shaderCompiler.h"
#include "../include/texture.h"

#define PI 4 * atan(1)

void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
  auto const src_str = [source]() {
    switch (source)
    {
    case GL_DEBUG_SOURCE_API: return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
    case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
    case GL_DEBUG_SOURCE_OTHER: return "OTHER";
    }
  }();

  auto const type_str = [type]() {
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR: return "ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
    case GL_DEBUG_TYPE_MARKER: return "MARKER";
    case GL_DEBUG_TYPE_OTHER: return "OTHER";
    }
  }();

  auto const severity_str = [severity]() {
    switch (severity) {
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
    case GL_DEBUG_SEVERITY_LOW: return "LOW";
    case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
    case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
    }
  }();
  std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
}

int main(int argc, char** argv) {

  if (argc != 2) {
    std::cerr << "Please give proper command line arguments to specify the absolute path to project root.\n";
    return -1;
  }
  std::string dir = argv[1];
  std::string slash = "/";
  if (dir.substr(dir.size() - 1, dir.size()) == slash)
    dir = dir.substr(0, dir.size() - 1);
  std::string dir1 = dir + "/assets/verification";
  std::ifstream directoryCheck(dir1.c_str());
  std::string checkString;
  std::getline(directoryCheck, checkString);
  directoryCheck.close();  
  if (checkString != "ROOT_VERIFY") {
      std::cerr << "You have failed!!! Enter the correct path next time (;\n";
      return -1;
  }
  
  int height = 800;
  int width = 1200;

  // Window
  glfwInit();

  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(width, height, "Voxel", NULL, NULL);
  if (!window) {
    std::cout << "Window creation failed successfully\n";
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  glfwSwapInterval(0);

  gladLoadGL();

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(message_callback, nullptr);

  // ImGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");

  std::string path = dir + "/";
  std::string pathToFont = path + "assets/fonts/PixelOperator8.ttf";
  float size_pixels = 16.0;
  ImFont *programFont =
      io.Fonts->AddFontFromFileTTF(pathToFont.c_str(), size_pixels);

  std::string pathToIni = path + "imgui.ini";
  io.IniFilename = pathToIni.c_str();

  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;


  Shader shaderProgramHUD(dir + "/shaders/vertexHUD.glsl", dir + "/shaders/fragmentHUD.glsl");

  GUIItem box;
  box.windowHeight = height;
  box.windowWidth = width;
  box.height = 1;
  box.width = 30;
  box.position = glm::ivec2(0, 0);
  box.color = glm::vec3(1.0, 1.0, 1.0);
  box.genBox();

  GUIItem box2;
  box2.windowHeight = height;
  box2.windowWidth = width;
  box2.height = 30;
  box2.width = 1;
  box2.position = glm::ivec2(0, 0);
  box2.color = glm::vec3(1.0, 1.0, 1.0);
  box2.genBox();

  HUD boxDraw;
  boxDraw.list.push_back(box);
  boxDraw.list.push_back(box2);
  boxDraw.generateMesh();

  std::vector<int> vertices{0, 1, 2};
  std::vector<unsigned int> ebo{0, 1, 2};

  VAO VAO1;
  VAO1.Bind();

  VBO VBO1;
  EBO EBO1;

  VBO1.Gen(vertices);
  EBO1.Gen(ebo);

  VAO1.LinkAttribPointer(VBO1, 0, 1, GL_SHORT, 1 * sizeof(int), (void *)0);

  VAO1.Unbind();

  double prevTime = glfwGetTime();
  double chunkTimer = glfwGetTime();

  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);

  // glEnable(GL_BLEND);


  VoxelGame voxel(width, height, glm::dvec3(7.0, 7.0, 7.0), dir);

  voxel.frustumCuller.screenHeight = height;
  voxel.frustumCuller.screenDiag = sqrt((height * height) + (width * width));
  voxel.frustumCuller.FOV = glm::radians(90.0);


  voxel.camera.breakBlock = std::bind(&PlayerChunkInterface::breakBlock, &voxel.interface);
  voxel.camera.placeBlock = std::bind(&PlayerChunkInterface::placeBlock, &voxel.interface);
  voxel.camera.currentBlockPtr = &voxel.interface.currentBlock;

  voxel.processManager.lighting.lightDataOnGPU.Gen(3, (RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE) * ((CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) * sizeof(float));


  std::thread calcLoaded(&ChunkProcessManager::calculateLoadedChunks, &voxel.processManager);
  std::thread assigner(&ChunkProcessManager::chunkPopulator, &voxel.processManager);
  std::thread generator(&ChunkProcessManager::generateChunks, &voxel.processManager);
  std::thread builder(&ChunkProcessManager::buildChunks, &voxel.processManager);

  int count = 0;

  glm::vec3 sunDir = glm::vec3(cos(std::numbers::pi / 3), sin(std::numbers::pi / 3), 0.0);
  float increment = 0.003;
  unsigned long counter = 1;
  int chunkCount = 0;

  // chunkLister.initDataSSBO();
  // chunkLister.dispatchCompute(0);

  std::cout << sizeof(VoxelGame) << "\n";

  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);

    voxel.frustumCuller.screenHeight = height;
    voxel.frustumCuller.screenDiag = sqrt((height * height) + (width * width));

    box.windowHeight = height;
    box.windowWidth = width;
    box2.windowHeight = height;
    box2.windowWidth = width;

    box.genBox();
    box2.genBox();
    boxDraw.generateMesh();

    voxel.camera.width = width;
    voxel.camera.height = height;

    glViewport(0, 0, width, height);

    glEnable(GL_CULL_FACE);

    voxel.camera.mouseInput(window);

    voxel.camera.inputs(window);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.5f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    double crntTime = glfwGetTime();
    int fps = 1 / (crntTime - prevTime);
    prevTime = crntTime;

    // sunDir.x = cos(counter * increment);
    // sunDir.y = sin(counter * increment);
    // counter++;

    voxel.highlightCursor.positionCursor();

    count++;

    voxel.renderer.regionCompileRoutine();

    int newChunkCount = 0;

    voxel.renderer.renderVoxelWorld();

    int increment;
    double timeDiff = glfwGetTime();
    int cps;
    if (timeDiff - chunkTimer > 1.0) {
      increment = newChunkCount - chunkCount;
      cps = increment / (timeDiff - chunkTimer);
      chunkTimer = timeDiff;
      chunkCount = newChunkCount;
    }

    glDisable(GL_CULL_FACE);

    voxel.highlightCursor.renderCursor();

    glDisable(GL_DEPTH_TEST);

    shaderProgramHUD.Activate();
    boxDraw.boxVAO.Bind();
    glDrawElements(GL_TRIANGLES, boxDraw.EBOsize, GL_UNSIGNED_INT, 0);
    boxDraw.boxVAO.Unbind();

    ImGui::PushFont(programFont);

    ImGui::Begin("FPS and Coordinates HUD", NULL,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::TextColored(ImVec4(0.6, 0.986, 0.53, 1.0), "Voxel Test");
    ImGui::Spacing();
    std::string fpsString = "FPS: " + std::to_string(fps);
    ImGui::Text("%s", fpsString.c_str());
    std::string loadedChunkString = "Loaded Chunks: " + std::to_string(newChunkCount) + " / " + std::to_string(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE);
    ImGui::Text("%s", loadedChunkString.c_str());
    std::string cpsString = "CPS: " + std::to_string(cps);
    ImGui::Text("%s", cpsString.c_str());
    ImGui::Spacing();
    std::string coordinateString = "XYZ: " + std::to_string(voxel.camera.Position.x) + " | " + std::to_string(voxel.camera.Position.y) + " | " + std::to_string(voxel.camera.Position.z);
    ImGui::Text("%s", coordinateString.c_str());
    int blockX = fastFloat::fastFloor(voxel.camera.Position.x);
    int blockY = fastFloat::fastFloor(voxel.camera.Position.y);
    int blockZ = fastFloat::fastFloor(voxel.camera.Position.z);
    std::string blockString = "Block: " + std::to_string(blockX) + " | " + std::to_string(blockY) + " | " + std::to_string(blockZ);
    ImGui::Text("%s", blockString.c_str());
    std::string chunkString =
        "Chunk: " +
        std::to_string(fastFloat::fastFloor(voxel.camera.Position.x / CHUNK_SIZE)) +
        " | " +
        std::to_string(fastFloat::fastFloor(voxel.camera.Position.y / CHUNK_SIZE)) +
        " | " +
        std::to_string(fastFloat::fastFloor(voxel.camera.Position.z / CHUNK_SIZE));
    ImGui::Text("%s", chunkString.c_str());
    std::string facingString =
        "Facing: " + std::to_string(voxel.camera.sphericalOrientation.x) + " | " +
        std::to_string(voxel.camera.sphericalOrientation.y);
    ImGui::Text("%s", facingString.c_str());
    if (voxel.highlightCursor.crntLookingAtBlock.blockPos !=
        glm::ivec3(2147483647, 2147483647, 2147483647)) {
      std::string blockPosString =
          "Looking At: " + std::to_string(voxel.highlightCursor.crntLookingAtBlock.blockPos.x) + " | " +
          std::to_string(voxel.highlightCursor.crntLookingAtBlock.blockPos.y) + " | " +
          std::to_string(voxel.highlightCursor.crntLookingAtBlock.blockPos.z);
      ImGui::Text("%s", blockPosString.c_str());
    }
    ImGui::End();

    ImGui::PopFont();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  voxel.processManager.run = 0;

  calcLoaded.join();
  assigner.join();
  generator.join();
  builder.join();

  // Deleting things
  // shaderProgram.Delete();
  // tex.Delete();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
