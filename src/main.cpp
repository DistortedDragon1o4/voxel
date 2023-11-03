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
#include "stb/stb_image.h"

#include "../include/HUD.h"
#include "../include/camera.h"
#include "../include/chunkList.h"
#include "../include/path.h"
#include "../include/shaderCompiler.h"
#include "../include/texture.h"

#define PI 4 * atan(1)

int main() {

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

  gladLoadGL();

  // ImGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");

  std::string path = PATH;
  std::string pathToFont = path + "assets/fonts/PixelOperator8.ttf";
  float size_pixels = 16.0;
  ImFont *programFont =
      io.Fonts->AddFontFromFileTTF(pathToFont.c_str(), size_pixels);

  std::string pathToIni = path + "imgui.ini";
  io.IniFilename = pathToIni.c_str();

  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

  /*ImVec4* colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
  colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
  colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
  colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
  colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
  colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
  colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
  colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
  colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
  colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
  colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
  colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
  colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
  colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
  colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
  colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
  colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
  colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
  colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
  colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
  colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
  colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
  colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
  colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
  colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
  colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
  colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  //colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f,
  0.86f, 1.00f);
  //colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f,
  0.00f, 1.00f); colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f,
  0.00f, 1.00f); colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f,
  0.00f, 1.00f); colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f,
  0.00f, 1.00f); colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f,
  0.00f, 1.00f); colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f,
  0.00f, 0.52f); colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f,
  0.00f, 0.52f); colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f,
  0.28f, 0.29f); colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f,
  0.00f, 0.00f); colors[ImGuiCol_TableRowBgAlt]          =
  ImVec4(1.00f, 1.00f, 1.00f, 0.06f); colors[ImGuiCol_TextSelectedBg]         =
  ImVec4(0.20f, 0.22f, 0.23f, 1.00f); colors[ImGuiCol_DragDropTarget]         =
  ImVec4(0.33f, 0.67f, 0.86f, 1.00f); colors[ImGuiCol_NavHighlight]           =
  ImVec4(1.00f, 0.00f, 0.00f, 1.00f); colors[ImGuiCol_NavWindowingHighlight]  =
  ImVec4(1.00f, 0.00f, 0.00f, 0.70f); colors[ImGuiCol_NavWindowingDimBg]      =
  ImVec4(1.00f, 0.00f, 0.00f, 0.20f); colors[ImGuiCol_ModalWindowDimBg]       =
  ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowPadding                     = ImVec2(8.00f, 8.00f);
  style.FramePadding                      = ImVec2(5.00f, 2.00f);
  style.CellPadding                       = ImVec2(6.00f, 6.00f);
  style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
  style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
  style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
  style.IndentSpacing                     = 25;
  style.ScrollbarSize                     = 15;
  style.GrabMinSize                       = 10;
  style.WindowBorderSize                  = 1;
  style.ChildBorderSize                   = 1;
  style.PopupBorderSize                   = 1;
  style.FrameBorderSize                   = 1;
  style.TabBorderSize                     = 1;
  style.WindowRounding                    = 7;
  style.ChildRounding                     = 4;
  style.FrameRounding                     = 3;
  style.PopupRounding                     = 4;
  style.ScrollbarRounding                 = 9;
  style.GrabRounding                      = 3;
  style.LogSliderDeadzone                 = 4;
  style.TabRounding                       = 4;*/

  // Shaders
  const std::string vertexCode =
#include "vertex.glsl"
      ;
  const std::string fragmentCode =
#include "fragment.glsl"
      ;
  // const std::string geometryCode =
  // #include "geometry.glsl"
  // ;

  Shader shaderProgram(vertexCode.c_str(), fragmentCode.c_str());

  TextureArray tex(GL_TEXTURE0, "blocks/", 1, NUM_BLOCKS);
  tex.TexUnit(shaderProgram, "array", 0);

  const std::string vertexCodeHighlight =
#include "vertexHighlight.glsl"
      ;
  const std::string fragmentCodeHighlight =
#include "fragmentHighlight.glsl"
      ;

  Shader shaderProgramHighlight(vertexCodeHighlight.c_str(),
                                fragmentCodeHighlight.c_str());

  // Shaders
  const std::string vertexCodeHUD =
#include "vertexHUD.glsl"
      ;
  const std::string fragmentCodeHUD =
#include "fragmentHUD.glsl"
      ;

  Shader shaderProgramHUD(vertexCodeHUD.c_str(), fragmentCodeHUD.c_str());

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
  std::vector<uint> ebo{0, 1, 2};

  VAO VAO1;
  VAO1.Bind();

  VBO VBO1;
  EBO EBO1;

  VBO1.Gen(vertices);
  EBO1.Gen(ebo);

  VAO1.LinkAttribPointer(VBO1, 0, 1, GL_SHORT, 1 * sizeof(int), (void *)0);

  VAO1.Unbind();

  // Uniforms
  int locChunkID = glGetUniformLocation(shaderProgram.ID, "chunkID");
  int locCamPos = glGetUniformLocation(shaderProgram.ID, "camPos");
  int locCamDir = glGetUniformLocation(shaderProgram.ID, "camDir");
  int locSunDir = glGetUniformLocation(shaderProgram.ID, "sunDir");
  int locIndex = glGetUniformLocation(shaderProgram.ID, "index");

  int locPos = glGetUniformLocation(shaderProgramHighlight.ID, "Pos");

  double prevTime = glfwGetTime();
  double chunkTimer = glfwGetTime();

  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);

  glEnable(GL_FOG);
  // glEnable(GL_BLEND);

  ChunkList chunkLister;

  Camera camera(width, height, glm::dvec3(0.0, 0.0, 0.0));

  camera.breakBlock = std::bind(&ChunkList::breakBlock, &chunkLister);
  camera.placeBlock = std::bind(&ChunkList::placeBlock, &chunkLister);
  camera.currentBlockPtr = &chunkLister.currentBlock;

  chunkLister.lightDataOnGPU.Gen(
      3, (RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE) *
             ((CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) *
             sizeof(float));

  chunkLister.blockInit();

  chunkLister.createHighlightVAO();

  chunkLister.camPosX = camera.Position.x;
  chunkLister.camPosY = camera.Position.y;
  chunkLister.camPosZ = camera.Position.z;

  std::thread calcLoaded(&ChunkList::calculateLoadedChunks, &chunkLister);

  std::thread assigner(&ChunkList::assignChunkID, &chunkLister);

  std::thread generator(&ChunkList::generateChunks, &chunkLister);

  std::thread builder0(&ChunkList::organiseChunks, &chunkLister, 0);
  // std::thread builder1(&ChunkList::organiseChunks, &chunkLister, 1);
  // std::thread builder2(&ChunkList::organiseChunks, &chunkLister, 2);
  // std::thread builder3(&ChunkList::organiseChunks, &chunkLister, 3);
  // std::thread builder4(&ChunkList::organiseChunks, &chunkLister, 4);
  // std::thread builder5(&ChunkList::organiseChunks, &chunkLister, 5);
  // std::thread builder6(&ChunkList::organiseChunks, &chunkLister, 6);
  // std::thread builder7(&ChunkList::organiseChunks, &chunkLister, 7);

  int count = 0;

  glm::vec3 sunDir =
      glm::vec3(cos(std::numbers::pi / 3), sin(std::numbers::pi / 3), 0.0);
  float increment = 0.003;
  unsigned long counter = 1;
  int chunkCount = 0;

  // chunkLister.initDataSSBO();
  // chunkLister.dispatchCompute(0);

  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);

    box.windowHeight = height;
    box.windowWidth = width;
    box2.windowHeight = height;
    box2.windowWidth = width;

    box.genBox();
    box2.genBox();
    boxDraw.generateMesh();

    camera.width = width;
    camera.height = height;

    glViewport(0, 0, width, height);

    glEnable(GL_CULL_FACE);

    camera.mouseInput(window);

    camera.inputs(window);

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

    chunkLister.camDir = camera.Orientation;

    chunkLister.camPosX = camera.Position.x;
    chunkLister.camPosY = camera.Position.y;
    chunkLister.camPosZ = camera.Position.z;
    chunkLister.rayCastTillBlock(camera.Orientation, camera.Position, 10.0);

    count++;

    chunkLister.putInVAOs();

    shaderProgram.Activate();
    // please remember to change FOV in the render calls frustum culling
    camera.matrix(90.0, 0.001, 512.0, shaderProgram, "cameraMatrix");
    tex.Bind();

    glUniform3fv(locCamPos, 1, &glm::vec3(camera.Position)[0]);
    glUniform3fv(locCamDir, 1, &glm::vec3(camera.Orientation)[0]);
    glUniform3fv(locSunDir, 1, &glm::vec3(sunDir)[0]);

    int newChunkCount = 0;

    for (int i = 0; i < chunkLister.chunkWorldContainer.size(); i++) {
      // please remember to FOV here when changing FOV
      if (chunkLister.chunkWorldContainer[i].renderlck == 0 &&
          chunkLister.chunkWorldContainer[i].unCompiledChunk == 0 &&
          chunkLister.chunkWorldContainer[i].EBOsize != 0) {
        newChunkCount++;
        chunkLister.chunkWorldContainer[i].array.Bind();
        glUniform3iv(locChunkID, 1, &chunkLister.chunkWorldContainer[i].chunkID[0]);
        glUniform1i(locIndex, i);
        glDrawElements(GL_TRIANGLES, chunkLister.chunkWorldContainer[i].EBOsize,GL_UNSIGNED_INT, 0);
        chunkLister.chunkWorldContainer[i].array.Unbind();
      }
    }

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

    shaderProgramHighlight.Activate();

    camera.matrix(90.0, 0.001, 512.0, shaderProgramHighlight, "cameraMatrix");
    glUniform3fv(locPos, 1, &glm::vec3(chunkLister.blockPos)[0]);

    chunkLister.highlightVAO.Bind();
    glDrawElements(GL_LINE_STRIP, chunkLister.EBOsize, GL_UNSIGNED_INT, 0);
    chunkLister.highlightVAO.Unbind();

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
    std::string loadedChunkString =
        "Loaded Chunks: " + std::to_string(newChunkCount) + " / " +
        std::to_string(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE);
    ImGui::Text("%s", loadedChunkString.c_str());
    std::string cpsString = "CPS: " + std::to_string(cps);
    ImGui::Text("%s", cpsString.c_str());
    ImGui::Spacing();
    std::string coordinateString = "XYZ: " + std::to_string(camera.Position.x) +
                                   " | " + std::to_string(camera.Position.y) +
                                   " | " + std::to_string(camera.Position.z);
    ImGui::Text("%s", coordinateString.c_str());
    int blockX = fastFloat::fastFloor(camera.Position.x);
    int blockY = fastFloat::fastFloor(camera.Position.y);
    int blockZ = fastFloat::fastFloor(camera.Position.z);
    std::string blockString = "Block: " + std::to_string(blockX) + " | " +
                              std::to_string(blockY) + " | " +
                              std::to_string(blockZ);
    ImGui::Text("%s", blockString.c_str());
    std::string chunkString =
        "Chunk: " +
        std::to_string(fastFloat::fastFloor(camera.Position.x / CHUNK_SIZE)) +
        " | " +
        std::to_string(fastFloat::fastFloor(camera.Position.y / CHUNK_SIZE)) +
        " | " +
        std::to_string(fastFloat::fastFloor(camera.Position.z / CHUNK_SIZE));
    ImGui::Text("%s", chunkString.c_str());
    std::string facingString =
        "Facing: " + std::to_string(camera.sphericalOrientation.x) + " | " +
        std::to_string(camera.sphericalOrientation.y);
    ImGui::Text("%s", facingString.c_str());
    if (chunkLister.blockPos !=
        glm::ivec3(2147483647, 2147483647, 2147483647)) {
      std::string blockPosString =
          "Looking At: " + std::to_string(chunkLister.blockPos.x) + " | " +
          std::to_string(chunkLister.blockPos.y) + " | " +
          std::to_string(chunkLister.blockPos.z);
      ImGui::Text("%s", blockPosString.c_str());
    }
    ImGui::End();

    ImGui::PopFont();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  chunkLister.run = 0;

  calcLoaded.join();
  assigner.join();
  generator.join();
  builder0.join();
  // builder1.join();
  // builder2.join();
  // builder3.join();
  // builder4.join();
  // builder5.join();
  // builder6.join();
  // builder7.join();

  // Deleting things
  shaderProgram.Delete();
  tex.Delete();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
