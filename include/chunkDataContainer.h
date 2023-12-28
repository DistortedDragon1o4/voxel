#ifndef CHUNK_DATA_CLASS_H
#define CHUNK_DATA_CLASS_H

#include <string>
#include <array>
#include <vector>
#include <bitset>
#include "VAO.h"
#include <coordinateContainers.h>

#define NUM_BLOCKS 6
#define RENDER_DISTANCE 24
#define CHUNK_SIZE 16

// Please DO NOT change this
#define REGION_SIZE 4
#define BUFFER_OFFSET(offset) (static_cast<char*>(0) + (offset))

#define ALLOC_1MB_OF_VERTICES 131072


struct ChunkDataContainer {
	// stores the blockID of each block in the chunk
	std::vector<short> chunkData;
	std::vector<float> lightData = std::vector<float>((CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1));
	std::vector<unsigned int> mesh;

	int meshSize = 0;
    bool redoRegionMesh = false;

    int myIndex;
	ChunkCoords chunkID;
	float distance;
    
	bool inQueue = 0;
    bool inBFSqueue = 0;

	bool inGeneratorQueue = 0;
    bool inGenBFSqueue = 0;

	bool emptyChunk = 1;
	bool unCompiledChunk = 1;
	bool unGeneratedChunk = 1;
	bool vaolck = 0;
	bool renderlck = 0;
	bool forUpdate = 0;

    bool frustumVisible = false;

    bool occlusionUnCulled = false;

    bool isPermeableCheckDone = 0;
    short permeability = 0;
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
    std::array<void*, REGION_SIZE * REGION_SIZE * REGION_SIZE> indexOffset = {BUFFER_OFFSET(0)};
    std::array<int, REGION_SIZE * REGION_SIZE * REGION_SIZE> baseVertex = {};

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

    bool entirelyCulled = false;
};


struct BlockTemplate {
    // vertex format:
    // | 4 bits for texture U | 4 bits for texture V | 8 bits for X | 8 bits for Y | 8 bits for Z |
    // | 28 bits for texture coordinate | 3 bits for normal | 1 bit for ambient occlusion |

    const std::vector<unsigned int> blockBitMap {
        //| U|| V||     X||     Y||     Z|
        0b10001000000010000000100000001000,      0b10100000000,
        0b00001000000000000000100000001000,      0b10100000000,
        0b00000000000000000000000000001000,      0b10100000000,
        0b10000000000010000000000000001000,      0b10100000000,
        //| U|| V||     X||     Y||     Z|
        0b10001000000000000000100000000000,      0b10000000000,
        0b00001000000010000000100000000000,      0b10000000000,
        0b00000000000010000000000000000000,      0b10000000000,
        0b10000000000000000000000000000000,      0b10000000000,
        //| U|| V||     X||     Y||     Z|
        0b10001000000010000000100000000000,      0b01100000000,
        0b00001000000010000000100000001000,      0b01100000000,
        0b00000000000010000000000000001000,      0b01100000000,
        0b10000000000010000000000000000000,      0b01100000000,
        //| U|| V||     X||     Y||     Z|
        0b10001000000000000000100000001000,      0b01000000000,
        0b00001000000000000000100000000000,      0b01000000000,
        0b00000000000000000000000000000000,      0b01000000000,
        0b10000000000000000000000000001000,      0b01000000000,
        //| U|| V||     X||     Y||     Z|
        0b10001000000010000000100000000000,      0b00100000000,
        0b00001000000000000000100000000000,      0b00100000000,
        0b00000000000000000000100000001000,      0b00100000000,
        0b10000000000010000000100000001000,      0b00100000000,
        //| U|| V||     X||     Y||     Z|
        0b10001000000010000000000000001000,      0b00000000000,
        0b00001000000000000000000000001000,      0b00000000000,
        0b00000000000000000000000000000000,      0b00000000000,
        0b10000000000010000000000000000000,      0b00000000000
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
};

#endif
