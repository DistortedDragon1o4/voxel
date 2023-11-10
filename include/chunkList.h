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
#include <bitset>
#include <ankerl/unordered_dense.h>

#include "buffers.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "shaderCompiler.h"

#define NUM_THREADS 1

struct Region {
	short neighbours = 0;  // We use the six bits to denote which faces of the chunk it contains, order is posX, negX, posY, negY, posZ, negZ from right to left
// std::vector<int> blockIndices;
};

union DoubleToLongLong {
    double d;
    long long ll;
    DoubleToLongLong(double _d) : d(_d) {}
};

class ChunkList {
private:
	std::array<Blocks, NUM_BLOCKS> blocks;

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

	int getIndex(const ChunkCoords chunkCoord);
	int getIndex(const int chunkCoordX, const int chunkCoordY, const int chunkCoordZ);

	bool atBit(const int value, const unsigned int position);
	int ambientOccIndex(int coordinates);

	bool checkVisibility(glm::vec3 pos, glm::vec3 camDir, double FOV);

	ankerl::unordered_dense::map<long, unsigned int> coordToIndexMap;

	long coordsToKey(const ChunkCoords coords);

	// std::vector<float> angleFromCamera = std::vector<float>((RENDER_DISTANCE +
	// 1) * (RENDER_DISTANCE + 1) * (RENDER_DISTANCE + 1));

	std::vector<short> mesh;
	std::vector<uint> indices;

	int chunkX[NUM_THREADS];
	int chunkY[NUM_THREADS];
	int chunkZ[NUM_THREADS];

	std::array<std::array<short, (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)>, NUM_THREADS> cachedBlocks = {-1};

	double increment = 0.05;

	glm::dvec3 previousCamPos;

	std::queue<int> chunkMeshingQueue;
	// std::queue<int> chunkGeneratingQueue;
	std::queue<ChunkCoords> BFSqueue;
	// std::queue<std::array<int, 3>> BFSqueueGenerator;
	void searchNeighbouringChunks(const ChunkCoords chunkID);
	void doBFS(const ChunkCoords chunk);
	// void searchNeighbouringChunksGenerator(std::array<int, 3> chunkID);
	// void doBFSGenerator(std::array<int, 3> chunk);

	std::array<bool, RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE> localOcclusionUnCulled = {0};

	std::mutex chunkWorldContainerMutex;

	std::vector<float> highlightCube {
	1.0001f,  1.0001f,  1.0001f,  -0.0001f, 1.0001f,  1.0001f,
	-0.0001f, -0.0001f, 1.0001f,  1.0001f,  -0.0001f, 1.0001f,
	1.0001f,  1.0001f,  -0.0001f, -0.0001f, 1.0001f,  -0.0001f,
	-0.0001f, -0.0001f, -0.0001f, 1.0001f,  -0.0001f, -0.0001f
	};
	std::vector<uint> highlightEBO{0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0, 1, 5, 6, 2, 3, 7, 4};



	void checkPermeability(ChunkDataContainer &chunk);
	std::queue<int> BFSqueuePermeability;
	// bool checkIfInRegion(std::vector<Region> &regions, int blockIndex);
	// bool checkIfInRegion(Region &region, int blockIndex);
	void doBlockBFSforPermeability(int startIndex, Region &region, ChunkDataContainer &chunk, std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> &chunkDataBFSvisited);
	void searchNeighbouringBlocks(int blockIndex, ChunkDataContainer &chunk, std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> &chunkDataBFSvisited);
	int getBlockIndex(int x, int y, int z);
	short facing(int index);
	short generatePermeability(Region &region);

	short permeabilityIndex(int a, int b);

	double cosineModifiedHalfFOV;
	double frustumOffset;

public:
	bool isFrustumCulled(const ChunkCoords &chunkCoords);

	double FOV;
	double screenDiag;
	double screenHeight;

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

	std::vector<ChunkDataContainer> chunkWorldContainer = std::vector<ChunkDataContainer>(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE);

	// void lookingAtBlock();
	void createHighlightVAO();

	void updateChunk(int ChunkX, int ChunkY, int ChunkZ, bool surroundings);

	void chunkManager();

	void calculateLoadedChunks();
	void assignChunkID();

	void organiseChunks(int threadID);
	void putInVAOs();
	void generateChunks();

	void updateLight(const ChunkCoords coords, int threadID);
	void uploadLight(int index);
	SSBO lightDataOnGPU;

	int buildChunk(int threadID);

	void blockInit();

	Compute cullingProgram;

	SSBO chunkDataOnGPU;
	void uploadData(int index);
	void initDataSSBO();
	void dispatchCompute(int index);

	void rayCastTillBlock(const glm::dvec3 ray, const glm::dvec3 position, const double limit);

	int currentBlock = 5;
	void breakBlock();
	void placeBlock();

	short crntPerm = 0;
};

#endif
