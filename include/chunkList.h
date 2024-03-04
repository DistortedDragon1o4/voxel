#ifndef CHUNK_LIST_CLASS_H
#define CHUNK_LIST_CLASS_H

#include "camera.h"
#include "coordinateContainers.h"
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
#include "texture.h"

#define NUM_THREADS 1

// P.S. restructure this entire file

struct BlockDefs {
	BlockDefs(std::string dir);

	void parseModel(std::vector<unsigned int> &model, std::vector<unsigned int> &blockBitMap, std::vector<int> &faceType);

	std::array<Blocks, NUM_BLOCKS> blocks;
	BlockTemplate solidBlock;

	// void blockInit();
};

struct WorldContainer {
	WorldContainer(Camera &camera) : camera(camera) {}

	Camera &camera;

	long coordsToKey(const ChunkCoords coords);
	ankerl::unordered_dense::map<long, unsigned int> coordToIndexMap;
	std::vector<ChunkDataContainer> chunks = std::vector<ChunkDataContainer>(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE);

	// The most important function

	int getIndex(const ChunkCoords chunkCoord);
	int getIndex(const int chunkCoordX, const int chunkCoordY, const int chunkCoordZ);

	// The weirdest function that somehow works
	bool isEdgeChunk(int coordX, int coordY, int coordZ);
};

// struct ChunkPermeability {
// 	ChunkPermeability(WorldContainer &worldContainer, BlockDefs &blocks) : worldContainer(worldContainer), blocks(blocks) {};

// 	WorldContainer &worldContainer;
// 	BlockDefs &blocks;

// 	void checkPermeability(ChunkDataContainer &chunk);
// 	std::queue<int> BFSqueuePermeability;
// 	void doBlockBFSforPermeability(int startIndex, short &neighbours, ChunkDataContainer &chunk, std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> &chunkDataBFSvisited);
// 	void searchNeighbouringBlocks(int blockIndex, ChunkDataContainer &chunk, std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> &chunkDataBFSvisited);
// 	int getBlockIndex(int x, int y, int z);
// 	short facing(int index);
// 	short generatePermeability(short &neighbours);

// 	short permeabilityIndex(int a, int b);
// };

// struct ChunkMeshingQueueGenerator {
// 	ChunkMeshingQueueGenerator(WorldContainer &worldContainer, ChunkPermeability &permeability) : worldContainer(worldContainer), permeability(permeability) {};

// 	WorldContainer &worldContainer;
// 	ChunkPermeability &permeability;

// 	bool crntMeshingQueue;

// 	std::array<std::queue<int>, 2> chunkMeshingQueue;
// 	std::queue<ChunkCoords> BFSqueue;

// 	std::array<bool, RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE> localOcclusionUnCulled = {0};


// 	void searchNeighbouringChunks(const ChunkCoords chunkID);
// 	void doBFS(const ChunkCoords chunk);
// };

// struct ChunkGeneratingQueueGenerator {
// 	ChunkGeneratingQueueGenerator(WorldContainer &worldContainer) : worldContainer(worldContainer) {};

// 	WorldContainer &worldContainer;

// 	bool crntGeneratingQueue;

// 	std::array<std::queue<int>, 2> chunkGeneratingQueue;
// 	std::queue<ChunkCoords> BFSqueue;

// 	void searchNeighbouringChunks(const ChunkCoords chunkID);
// 	void doBFS(const ChunkCoords chunk);
// };

// struct MeshingQueue {
// 	MeshingQueue(WorldContainer &worldContainer) : worldContainer(worldContainer) {};

// 	WorldContainer &worldContainer;

// 	bool crntQueue = 0;

// 	std::array<std::queue<unsigned int>, 2> queue;

// 	std::queue<unsigned int> BFSqueue;

// 	void searchNeighbouringChunks(ChunkDataContainer &chunk);
// 	void doBFS(const ChunkCoords chunkCoord);
// };

// struct GeneratingQueue {
// 	GeneratingQueue(WorldContainer &worldContainer) : worldContainer(worldContainer) {};

// 	WorldContainer &worldContainer;

// 	bool crntQueue = 0;

// 	std::array<std::queue<unsigned int>, 2> queue;

// 	std::queue<unsigned int> BFSqueue;

// 	void searchNeighbouringChunks(ChunkDataContainer &chunk);
// 	void doBFS(const ChunkCoords chunkCoord);
// };

struct ChunkBuilder {
	ChunkBuilder(WorldContainer &worldContainer, BlockDefs &blocks) : worldContainer(worldContainer), blocks(blocks) {};

	WorldContainer &worldContainer;
	BlockDefs &blocks;

	bool discardChunk = 0;

	std::array<short, (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)> cachedBlocks = {-1};
	

	int buildChunk(ChunkDataContainer &chunk);

	void combineFace(int coordX, int coordY, int coordZ, ChunkDataContainer &chunk);
	int blockAt(int coordX, int coordY, int coordZ, ChunkDataContainer &chunk);
	int blockAtNeighbouringChunk(int coordX, int coordY, int coordZ, ChunkDataContainer &crntChunk);

	int cachedBlockAt(int coordX, int coordY, int coordZ);
	int ambientOccIndex(int coordinates);
};

struct LightingBFSQueueItem {
	ChunkDataContainer &chunk;
    BlockCoords coords;
    int distance;
};

struct ChunkLighting {
	ChunkLighting(WorldContainer &worldContainer, BlockDefs &blocks);

	WorldContainer &worldContainer;
	BlockDefs &blocks;

	UnifiedGLBufferContainer lightDataBuffer;

	void updateLight(ChunkDataContainer &chunk);
	void uploadLight(int index);
	void lightAO(ChunkDataContainer &chunk);

	void propagateLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunk);
	bool setLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunks);

	void depropagateLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunk);
	bool removeLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunks);

	std::queue<LightingBFSQueueItem> BFSqueue;
	std::vector<unsigned int> visitedChunkIndices;

	void addNeighboursToQueue(const BlockCoords coords, int distance, ChunkDataContainer &chunk);
	int getBlockIndex(int x, int y, int z);
};

struct RayCastReturn {
	glm::ivec3 blockPos;
	glm::ivec3 prevBlockPos;
	int blockPosIndex;
	int prevBlockPosIndex;

	int lightVal;
};

struct RayCaster {
	RayCaster(WorldContainer &worldContainer) : worldContainer(worldContainer) {};

	WorldContainer &worldContainer;

	RayCastReturn rayCastTillBlock(const glm::dvec3 ray, const glm::dvec3 position, const double limit);
};

struct HighlightCursor {
	HighlightCursor(RayCaster &rayCaster, Camera &camera, std::string dir) : rayCaster(rayCaster), camera(camera), shaderProgramHighlight(dir + "/shaders/vertexHighlight.glsl", dir + "/shaders/fragmentHighlight.glsl") {
		createHighlightVAO();
		locPos = glGetUniformLocation(shaderProgramHighlight.ID, "Pos");
		locCamPos = glGetUniformLocation(shaderProgramHighlight.ID, "camPos");
	};

	Shader shaderProgramHighlight;
	int locPos;
	int locCamPos;

	Camera &camera;
	RayCaster &rayCaster;

	std::vector<float> highlightCube {
		1.0001f,  1.0001f,  1.0001f,  -0.0001f, 1.0001f,  1.0001f,
		-0.0001f, -0.0001f, 1.0001f,  1.0001f,  -0.0001f, 1.0001f,
		1.0001f,  1.0001f,  -0.0001f, -0.0001f, 1.0001f,  -0.0001f,
		-0.0001f, -0.0001f, -0.0001f, 1.0001f,  -0.0001f, -0.0001f
	};
	std::vector<unsigned int> highlightEBO{
		0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0, 1, 5, 6, 2, 3, 7, 4
	};
	
	VAO highlightVAO;
	int EBOsize = 36;

	RayCastReturn crntLookingAtBlock;

	void positionCursor();
	void createHighlightVAO();
	void renderCursor();
};


// The ChunkCoords object is used because the coordinates of the region will be on the same scale as the chunks
// A region will be identified by the chunk it can contain at (0, 0, 0) relative to it
struct RegionContainer {
	long coordsToKey(const ChunkCoords coords);
	ankerl::unordered_dense::map<long, unsigned int> coordToIndexMap;
	std::vector<Region> regions = std::vector<Region>(((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2));
	// std::vector<Region> regions = std::vector<Region>((RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE) / 8);

	int getIndex(const ChunkCoords chunkCoord);
	int getIndex(const int regionCoordX, const int regionCoordY, const int regionCoordZ);
};

// A lot of work pending
struct ChunkProcessManager {
	ChunkProcessManager(WorldContainer &worldContainer, BlockDefs &blocks, RegionContainer &regionContainer, Camera &camera);

	Camera &camera;

	WorldContainer &worldContainer;
	BlockDefs &blocks;

	ChunkBuilder builder;
	ChunkLighting lighting;

	ChunkGen generator;


	RegionContainer &regionContainer;

	// Future stuff
	void initChunkRoutines();
	void destroyChunkRoutines();
	
	// The three horsemen
	void chunkPopulator();
	void buildChunks();
	void generateChunks();

	bool firstRun = 0;

	// The fourth horseman who does not get credits
	void updateChunk(ChunkCoords chunkCoords, bool surroundings);
	std::queue<int> chunkUpdateQueue;

	void calculateLoadedChunks();

	ChunkCoords prevCameraChunk;

	bool run = 1;

	// Everything after this has a weird reason to exist, the reason is to be found at a later date

	bool organiselck = 0;
};

struct Renderer {
	Renderer(Shader &voxelShader, TextureArray &voxelBlockTextureArray, Sampler &voxelBlockTextureSampler, Camera &camera, WorldContainer &worldContainer, RegionContainer &regionContainer, ChunkLighting &lighting, BlockDefs &blocks, std::string dir);

	newVAO voxelWorldVertexArray;

	Shader &voxelShader;
	TextureArray &voxelBlockTextureArray;
	Sampler &voxelBlockTextureSampler;


	Compute frustumCuller;
	int locCamPosFRC;
	int locCamDirFRC;
	int locFrustumOffset;
	int locCosineModifiedHalfFOV;

	double FOV;
	double screenDiag;
	double screenHeight;

	Compute orderingDrawCalls;
	Compute drawCallConstructor;

	ChunkLighting &lighting;

	BlockDefs &blocks;

	int locCamPos;
	int locCamDir;
	int locSunDir;

    newIBO ibo;

    UnifiedGLBufferContainer commandBuffer;
    UnifiedGLBufferContainer alternateCommandBuffer;

    UnifiedGLBufferContainer chunkViewableBuffer;
    UnifiedGLBufferContainer regionDataBuffer;

    newVBO copyBuffer;

    int oldMaxQuads = 0;
    int maxQuads = 0;
    std::vector<unsigned int> indices;

    // Don't ask why this exists
    glm::vec3 sunDir = glm::vec3(cos(std::numbers::pi / 3), sin(std::numbers::pi / 3), 0.0);

	Camera &camera;

	WorldContainer &worldContainer;

	RegionContainer &regionContainer;

	void regionCompileRoutine();
	void uploadMeshes(Region &region);
	void populateRegionData(Region &region, int index);

	// TODO
	// void uploadLighting();

	GLsync chunkVisibleArrSync;

	void preRenderVoxelWorld();
	void renderVoxelWorld();

	void generateIBO();

	bool run = true;
};

struct PlayerChunkInterface {
	PlayerChunkInterface(WorldContainer &worldContainer, ChunkProcessManager &processManager, HighlightCursor &highlightCursor) : worldContainer(worldContainer), processManager(processManager), highlightCursor(highlightCursor) {};

	WorldContainer &worldContainer;
	ChunkProcessManager &processManager;
	HighlightCursor &highlightCursor;

	int currentBlock = 5;
	void breakBlock();
	void placeBlock();
};

struct VoxelGame {
	VoxelGame(const int width, const int height, const glm::dvec3 position, const std::string dir);
	~VoxelGame();

	Shader voxelShader;
	TextureArray voxelBlockTextureArray;
	Sampler voxelBlockTextureSampler;

	Camera camera;

	BlockDefs blocks;
	WorldContainer worldContainer;

	RayCaster rayCaster;
	HighlightCursor highlightCursor;

	RegionContainer regionContainer;
	
	ChunkProcessManager processManager;

	PlayerChunkInterface interface;

	Renderer renderer;
};

#endif
