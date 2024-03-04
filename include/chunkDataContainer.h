#ifndef CHUNK_DATA_CLASS_H
#define CHUNK_DATA_CLASS_H

#include <string>
#include <array>
#include <vector>
#include <queue>
#include <bitset>
#include "VAO.h"
#include <coordinateContainers.h>


#define NUM_BLOCKS 6
#define NUM_TEXTURES 8
#define RENDER_DISTANCE 12
#define CHUNK_SIZE 32

// Please DO NOT change this
#define REGION_SIZE 4
#define BUFFER_OFFSET(offset) (static_cast<char*>(0) + (offset))

#define ALLOC_1MB_OF_VERTICES 131072

struct ChunkLightContainer {
    void clear() {for(unsigned int &i : data) i = 0;};
    std::array<unsigned int, (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)> data;
};

struct ChunkDataContainer {
	// stores the blockID of each block in the chunk
	std::vector<short> chunkData;
	
    std::vector<unsigned int> mesh;

    ChunkLightContainer lightData;
    // std::queue<LightUpdateInstruction> lightUpdateInstructions;
    std::vector<LightUpdateInstruction> lightUpdateInstructions;
    std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> chunkLightDataBFSvisited = {0};
    bool lightVisited = false;

	int meshSize = 0;
    bool redoRegionMesh = false;

    std::array<int, 27> neighbouringChunkIndices = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

	ChunkCoords chunkID;
	float distance;

	bool emptyChunk = 1;
	bool unCompiledChunk = 1;
	bool unGeneratedChunk = 1;
	bool renderlck = 0;
	bool forUpdate = 0;

    bool uploadLightAnyway = false;

    bool inMeshingBFSqueue = false;

    bool isSingleBlock = false;
    short theBlock = 0;

    short blockAtCoords(const BlockCoords coords);
    short blockAtCoords(const int _x, const int _y, const int _z);
    void setBlockAtCoords(const BlockCoords coords, const short block);
    void setBlockAtCoords(const int _x, const int _y, const int _z, const short block);

    void compressChunk();
};

struct Region {
    ~Region() {
        vbo.del();
    }

    // This is a container for 4x4x4 chunks
    std::array<ChunkDataContainer*, REGION_SIZE * REGION_SIZE * REGION_SIZE> chunksInRegion;

    // Does literally the opposite of its name    
    std::bitset<REGION_SIZE * REGION_SIZE * REGION_SIZE> isChunkNull = 0;

    ChunkCoords regionID;

    std::vector<unsigned int> mesh;

    int numFilledVertices = 0;

    std::array<int, REGION_SIZE * REGION_SIZE * REGION_SIZE> count = {};
    std::array<int, REGION_SIZE * REGION_SIZE * REGION_SIZE> baseVertex = {};

    unsigned int numFilledChunks = 0;

    // very misleading name, but ok
    // RegionProperties regionProperties;

    RegionDataContainer regionData;

    newVBO vbo;

    void compileMesh();
    void uploadRegion();

    bool shouldCompile = false;
    bool ready = false;
    bool readyToUpload = false;

    bool empty = true;

};


struct BlockTemplate {
    // vertex format:
    // | 10 bits for X | 10 bits for Y | 10 bits for Z |
    // | 10 bits for texture index | 5 bits for texture U | 5 bits for texture V | 3 bits for normal | 8 bit for ambient occlusion |

    const std::vector<unsigned int> blockBitMap {
        //|       X||       Y||       Z|         |  U||  V||N|     AO|
        0b000001000000000100000000010000,      0b100001000010100000000,
        0b000000000000000100000000010000,      0b000001000010100000000,
        0b000000000000000000000000010000,      0b000000000010100000000,
        0b000001000000000000000000010000,      0b100000000010100000000,
        //|       X||       Y||       Z|
        0b000000000000000100000000000000,      0b100001000010000000000,
        0b000001000000000100000000000000,      0b000001000010000000000,
        0b000001000000000000000000000000,      0b000000000010000000000,
        0b000000000000000000000000000000,      0b100000000010000000000,
        //|       X||       Y||       Z|
        0b000001000000000100000000000000,      0b100001000001100000000,
        0b000001000000000100000000010000,      0b000001000001100000000,
        0b000001000000000000000000010000,      0b000000000001100000000,
        0b000001000000000000000000000000,      0b100000000001100000000,
        //|       X||       Y||       Z|
        0b000000000000000100000000010000,      0b100001000001000000000,
        0b000000000000000100000000000000,      0b000001000001000000000,
        0b000000000000000000000000000000,      0b000000000001000000000,
        0b000000000000000000000000010000,      0b100000000001000000000,
        //|       X||       Y||       Z|
        0b000001000000000100000000000000,      0b100001000000100000000,
        0b000000000000000100000000000000,      0b000001000000100000000,
        0b000000000000000100000000010000,      0b000000000000100000000,
        0b000001000000000100000000010000,      0b100000000000100000000,
        //|       X||       Y||       Z|
        0b000001000000000000000000010000,      0b100001000000000000000,
        0b000000000000000000000000010000,      0b000001000000000000000,
        0b000000000000000000000000000000,      0b000000000000000000000,
        0b000001000000000000000000000000,      0b100000000000000000000
    };

    const std::vector<int> faceType {
        5,
        4,
        3,
        2,
        1,
        0
    };

    const std::array<unsigned int, 6> dataEBO {
        0, 1, 2,
    	2, 3, 0,
    };
};

struct Blocks {
    std::string blockName;
	bool isSolid = 1;
    bool castsAO = 1;
    std::vector<unsigned int> model;
    std::vector<unsigned int> blockBitMap;
    std::vector<int> faceType;
};

#endif
