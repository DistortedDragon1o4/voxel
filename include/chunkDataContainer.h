#ifndef CHUNK_DATA_CLASS_H
#define CHUNK_DATA_CLASS_H

#include <string>
#include <array>
#include <vector>
#include <bitset>
#include "VAO.h"
#include <coordinateContainers.h>

#define NUM_BLOCKS 6
#define RENDER_DISTANCE 12
#define CHUNK_SIZE 32

// Please DO NOT change this
#define REGION_SIZE 4
#define BUFFER_OFFSET(offset) (static_cast<char*>(0) + (offset))

#define ALLOC_1MB_OF_VERTICES 131072

struct ChunkLightContainer {
    std::array<unsigned int, (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)> data;// = std::vector<unsigned int>((CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2));
};

struct ChunkDataContainer {
	// stores the blockID of each block in the chunk
	std::vector<short> chunkData;
	
    std::vector<unsigned int> mesh;

    ChunkLightContainer lightData;

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

    // bool lightUploaded = false;
    // bool lightReadyToUpload = false;

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
    // | 13 bits for texture coordinate | 4 bits for texture U | 4 bits for texture V | 3 bits for normal | 8 bit for ambient occlusion |

    const std::vector<unsigned int> blockBitMap {
        //|       X||       Y||       Z|         | U|| V||N|     AO|
        0b000001000000000100000000010000,      0b1000100010100000000,
        0b000000000000000100000000010000,      0b0000100010100000000,
        0b000000000000000000000000010000,      0b0000000010100000000,
        0b000001000000000000000000010000,      0b1000000010100000000,
        //|       X||       Y||       Z|
        0b000000000000000100000000000000,      0b1000100010000000000,
        0b000001000000000100000000000000,      0b0000100010000000000,
        0b000001000000000000000000000000,      0b0000000010000000000,
        0b000000000000000000000000000000,      0b1000000010000000000,
        //|       X||       Y||       Z|
        0b000001000000000100000000000000,      0b1000100001100000000,
        0b000001000000000100000000010000,      0b0000100001100000000,
        0b000001000000000000000000010000,      0b0000000001100000000,
        0b000001000000000000000000000000,      0b1000000001100000000,
        //|       X||       Y||       Z|
        0b000000000000000100000000010000,      0b1000100001000000000,
        0b000000000000000100000000000000,      0b0000100001000000000,
        0b000000000000000000000000000000,      0b0000000001000000000,
        0b000000000000000000000000010000,      0b1000000001000000000,
        //|       X||       Y||       Z|
        0b000001000000000100000000000000,      0b1000100000100000000,
        0b000000000000000100000000000000,      0b0000100000100000000,
        0b000000000000000100000000010000,      0b0000000000100000000,
        0b000001000000000100000000010000,      0b1000000000100000000,
        //|       X||       Y||       Z|
        0b000001000000000000000000010000,      0b1000100000000000000,
        0b000000000000000000000000010000,      0b0000100000000000000,
        0b000000000000000000000000000000,      0b0000000000000000000,
        0b000001000000000000000000000000,      0b1000000000000000000
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
