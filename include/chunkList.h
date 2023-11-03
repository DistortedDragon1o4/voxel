#ifndef CHUNK_LIST_CLASS_H
#define CHUNK_LIST_CLASS_H

#include "fastFloat.h"
#include "glm/glm.hpp"
#include <bits/stdc++.h>
#include <chrono>
#include <iostream>
#include <math.h>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "buffers.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "shaderCompiler.h"

#define NUM_THREADS 1

class ChunkList {
private:
  Blocks blocks[NUM_BLOCKS];

  uint call[NUM_THREADS] = {0};
  bool discardChunk[NUM_THREADS] = {0};

  bool organiselck = 0;

  bool isInRenderedArea(const std::array<int, 3> &coords);

  bool firstRun = 0;
  int index[NUM_THREADS];

  ChunkGen generator;

  BlockTemplate solidBlock;

  int loadedChunkCoord[RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE][5];
  float chunkDistance[RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE];
  std::vector<int> updateQueue;

  bool alreadyIn(std::vector<int> queue, int element);

  // functions associated with chunk building
  void combineFace(int coordX, int coordY, int coordZ, int blockID, int threadID);
  int blockAt(int coordX, int coordY, int coordZ, int threadID);
  int globalBlockAt(int coordX, int coordY, int coordZ, int threadID);
  int cachedBlockAt(int coordX, int coordY, int coordZ, int threadID);
  void doIndices(int threadID);
  int getIndex(int coordX, int coordY, int coordZ);
  bool atBit(const int value, const unsigned int position);
  int ambientOccIndex(int coordinates);

  bool checkVisibility(glm::vec3 pos, glm::vec3 camDir, double FOV);

  std::unordered_map<std::string, unsigned int> coordToIndexMap;

  std::string coordsToString(std::array<int, 3> &coords);

  // std::vector<float> angleFromCamera = std::vector<float>((RENDER_DISTANCE +
  // 1) * (RENDER_DISTANCE + 1) * (RENDER_DISTANCE + 1));

  std::vector<short> mesh;
  std::vector<uint> indices;

  int chunkX[NUM_THREADS];
  int chunkY[NUM_THREADS];
  int chunkZ[NUM_THREADS];

  short cachedBlocks[NUM_THREADS]
                    [(CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)];

  double increment = 0.05;

  glm::dvec3 previousCamPos;

  std::queue<int> chunkMeshingQueue;
  std::queue<std::array<int, 3>> BFSqueue;
  void searchNeighbouringChunks(std::array<int, 3> chunkID);
  void doBFS(std::array<int, 3> chunk);

  std::mutex chunkWorldContainerMutex;

  std::vector<float> highlightCube{
      1.0001f,  1.0001f,  1.0001f,  -0.0001f, 1.0001f,  1.0001f,
      -0.0001f, -0.0001f, 1.0001f,  1.0001f,  -0.0001f, 1.0001f,
      1.0001f,  1.0001f,  -0.0001f, -0.0001f, 1.0001f,  -0.0001f,
      -0.0001f, -0.0001f, -0.0001f, 1.0001f,  -0.0001f, -0.0001f};
  std::vector<uint> highlightEBO{0, 1, 2, 3, 0, 4, 5, 6, 7,
                                 4, 0, 1, 5, 6, 2, 3, 7, 4};

public:
  double camPosX = 0.0;
  double camPosY = 0.0;
  double camPosZ = 0.0;

  glm::dvec3 camDir;

  glm::ivec3 blockPos;
  glm::ivec3 prevBlockPos;
  int blockPosIndex;
  int prevBlockPosIndex;

  VAO highlightVAO;
  int EBOsize = 36;

  bool run = 1;

  bool isEdgeChunk(int coordX, int coordY, int coordZ);

  std::vector<ChunkDataContainer> chunkWorldContainer =
      std::vector<ChunkDataContainer>(RENDER_DISTANCE * RENDER_DISTANCE *
                                      RENDER_DISTANCE);

  // void lookingAtBlock();
  void createHighlightVAO();

  void updateChunk(int ChunkX, int ChunkY, int ChunkZ, bool surroundings);

  void chunkManager();

  void calculateLoadedChunks();
  void assignChunkID();

  void organiseChunks(int threadID);
  void putInVAOs();
  void generateChunks();

  void updateLight(std::array<int, 3> &coords, int threadID);
  void uploadLight(int index);
  SSBO lightDataOnGPU;

  int buildChunk(int threadID);

  void blockInit();

  Compute cullingProgram;

  SSBO chunkDataOnGPU;
  void uploadData(int index);
  void initDataSSBO();
  void dispatchCompute(int index);

  void rayCastTillBlock(const glm::dvec3 ray, const glm::dvec3 position,
                        const double limit);

  int currentBlock = 5;
  void breakBlock();
  void placeBlock();
};

#endif
