#pragma once

#include <string>
#include <array>
#include <vector>
#include <queue>
#include <bitset>
#include "VAO.h"
#include "coordinateContainers.h"

#define NUM_BLOCKS 6
#define NUM_TEXTURES 8
// #define RENDER_DISTANCE 16
#define CHUNK_SIZE 32

#define REGION_SIZE 16

#define BUFFER_OFFSET(offset) (static_cast<char*>(0) + (offset))

#define ALLOC_1MB_OF_VERTICES 131072

struct ChunkLightContainer {
    // void clear() {for(unsigned int &i : data) i = 0;};
    // std::array<unsigned int, (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)> data;

    void init() {data = std::vector<unsigned int>((CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2));}
    void clear() {data.clear();}
    bool isEmpty = true;
    std::vector<unsigned int> data;

    void compress(std::vector<unsigned int> &compressedData);
};

struct Layer {
    Layer(const short _theBlock) {theBlock = _theBlock;};
    Layer() {blockData = std::vector<short>(CHUNK_SIZE * CHUNK_SIZE);};

    bool isSingleBlock = true;
    short theBlock = 0;
    std::vector<short> blockData;

    void setBlockAtCoords(const int _x, const int _z, const short block);

    void compressLayer();
};

struct ChunkData {
    void clear();
    void raw(std::vector<short> &rawBlockData);

    int lodLevel = 0;

    bool isSingleBlock = true;
    short theBlock = 0;
    std::vector<Layer> layerData;

    short blockAtCoords(const BlockCoords coords);
    short blockAtCoords(const int _x, const int _y, const int _z);
    void setBlockAtCoords(const BlockCoords coords, const short block);
    void setBlockAtCoords(const int _x, const int _y, const int _z, const short block);

    short blockAtCoords(const BlockCoords coords, const int LODlevel);
    short blockAtCoords(const int _x, const int _y, const int _z, const int LODlevel);

    void compressChunk();
};

struct ChunkDataContainer {
    unsigned int chunkType = 1;

	// stores the blockID of each block in the chunk
    ChunkData chunkData;
	
    std::vector<unsigned int> mesh;

    ChunkLightContainer lightData;
    std::vector<LightUpdateInstruction> lightUpdateInstructions;
    std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> chunkLightDataBFSvisited = {0};
    bool uploadLightAnyway = false;
    bool lightVisited = false;

	int meshSize = 0;
    bool reUploadMesh = false;

    bool reUploadLight = false;

    std::array<int, 27> neighbouringChunkIndices = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

	ChunkCoords chunkID;
	float distance = 0.0;
    float euclideanDistSquared = 0.0;

    bool contentsAreUseless = false;

	bool emptyChunk = true;
	bool unCompiledChunk = true;
	bool unGeneratedChunk = true;
    bool lodRecompilation = false;
	// bool renderlck = 0;
	bool forUpdate = false;

    bool meshLivesOnGPU = false;
    bool lightingLivesOnGPU = false;

    bool meshIsUseless = false;
    bool lightingIsUseless = false;
};

struct MiniChunkDataContainer {
    ChunkData chunkData;
    ChunkLightContainer lightData;
    ChunkCoords chunkID;
};


struct BlockTemplate {
    // vertex format:
    // | 10 bits for X | 10 bits for Y | 10 bits for Z |
    // | 10 bits for texture index | 5 bits for texture U | 5 bits for texture V | 3 bits for normal | 8 bit for ambient occlusion |

    const std::vector<unsigned int> blockBitMap {
        //|       X||       Y||       Z|         |  U||  V||N|AO|
        0b000001000000000100000000010000,      0b10000100001010,
        0b000000000000000100000000010000,      0b00000100001010,
        0b000000000000000000000000010000,      0b00000000001010,
        0b000001000000000000000000010000,      0b10000000001010,
        //|       X||       Y||       Z|
        0b000000000000000100000000000000,      0b10000100001000,
        0b000001000000000100000000000000,      0b00000100001000,
        0b000001000000000000000000000000,      0b00000000001000,
        0b000000000000000000000000000000,      0b10000000001000,
        //|       X||       Y||       Z|
        0b000001000000000100000000000000,      0b10000100000110,
        0b000001000000000100000000010000,      0b00000100000110,
        0b000001000000000000000000010000,      0b00000000000110,
        0b000001000000000000000000000000,      0b10000000000110,
        //|       X||       Y||       Z|
        0b000000000000000100000000010000,      0b10000100000100,
        0b000000000000000100000000000000,      0b00000100000100,
        0b000000000000000000000000000000,      0b00000000000100,
        0b000000000000000000000000010000,      0b10000000000100,
        //|       X||       Y||       Z|
        0b000001000000000100000000000000,      0b10000100000010,
        0b000000000000000100000000000000,      0b00000100000010,
        0b000000000000000100000000010000,      0b00000000000010,
        0b000001000000000100000000010000,      0b10000000000010,
        //|       X||       Y||       Z|
        0b000001000000000000000000010000,      0b10000100000000,
        0b000000000000000000000000010000,      0b00000100000000,
        0b000000000000000000000000000000,      0b00000000000000,
        0b000001000000000000000000000000,      0b10000000000000
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
