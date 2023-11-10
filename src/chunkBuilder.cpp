#include "../include/chunkList.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "fastFloat.h"
#include <thread>
#include <vector>

void ChunkList::doIndices(int threadID) {
    std::array<uint, 6> crntEBO = solidBlock.dataEBO;
    for (int i = 0; i < 6; i++) {
        crntEBO[i] += call[threadID];
    }
    chunkWorldContainer[index[threadID]].indices.insert(chunkWorldContainer[index[threadID]].indices.end(), crntEBO.begin(), crntEBO.end());
    call[threadID] += 4;
}

long ChunkList::coordsToKey(const ChunkCoords coords) {
    long result = (fastFloat::mod(coords.x, 2 * RENDER_DISTANCE) * 4 * RENDER_DISTANCE * RENDER_DISTANCE) + (fastFloat::mod(coords.y, 2 * RENDER_DISTANCE) * 2 * RENDER_DISTANCE) + fastFloat::mod(coords.z, 2 * RENDER_DISTANCE);
    return result;
}

int ChunkList::getIndex(const ChunkCoords chunkCoord) {
    int index;
    try {
        index = coordToIndexMap.at(coordsToKey(chunkCoord));
    } catch (const std::out_of_range& oor) {
        //std::cerr << "Out of Range error: " << oor.what() << "\n";
        return -1;
    }
    return index;
}

int ChunkList::getIndex(const int chunkCoordX, const int chunkCoordY, const int chunkCoordZ) {
    ChunkCoords chunkCoord;
    chunkCoord.x = chunkCoordX;
    chunkCoord.y = chunkCoordY;
    chunkCoord.z = chunkCoordZ;
    int index;
    try {
        index = coordToIndexMap.at(coordsToKey(chunkCoord));
    } catch (const std::out_of_range& oor) {
        //std::cerr << "Out of Range error: " << oor.what() << "\n";
        return -1;
    }
    return index;
}

int ChunkList::globalBlockAt(int coordX, int coordY, int coordZ, int threadID) {
    ChunkCoords currentChunk;
    currentChunk.x = chunkX[threadID];
    currentChunk.y = chunkY[threadID];
    currentChunk.z = chunkZ[threadID];
    int x = coordX;
    int y = coordY;
    int z = coordZ;
    if (coordX < 0) {
        currentChunk.x = chunkX[threadID] - 1;
        x = CHUNK_SIZE - 1;
    } else if (coordX >= CHUNK_SIZE) {
        currentChunk.x = chunkX[threadID] + 1;
        x = 0;
    }
    if (coordY < 0) {
        currentChunk.y = chunkY[threadID] - 1;
        y = CHUNK_SIZE - 1;
    } else if (coordY >= CHUNK_SIZE) {
        currentChunk.y = chunkY[threadID] + 1;
        y = 0;
    }
    if (coordZ < 0) {
        currentChunk.z = chunkZ[threadID] - 1;
        z = CHUNK_SIZE - 1;
    } else if (coordZ >= CHUNK_SIZE) {
        currentChunk.z = chunkZ[threadID] + 1;
        z = 0;
    }

    bool match = 0;
    int currentIndex = getIndex(currentChunk);

    if (currentIndex >= 0 && currentIndex < chunkWorldContainer.size() && chunkWorldContainer.at(currentIndex).chunkData.size() == CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) {
        try {
            return chunkWorldContainer.at(currentIndex).chunkData.at((x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + z);
        } catch (const std::out_of_range& oor) {
            std::cerr << "Out of Range error: " << oor.what() << "\n";
            discardChunk[threadID] = 1;
            return 0;
        }
    } else {
        discardChunk[threadID] = 1;
    }
    return 0;
}

int ChunkList::cachedBlockAt(int coordX, int coordY, int coordZ, int threadID) {
    return cachedBlocks[threadID][(coordX * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + (coordY * (CHUNK_SIZE + 2)) + coordZ];
}

int ChunkList::blockAt(int coordX, int coordY, int coordZ, int threadID) {
    int block = cachedBlockAt(coordX + 1, coordY + 1, coordZ + 1, threadID);
    if (block == -1) {
        block = globalBlockAt(coordX, coordY, coordZ, threadID);
        cachedBlocks[threadID][((coordX + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((coordY + 1) * (CHUNK_SIZE + 2)) + (coordZ + 1)] = block;
    }
    return block;
}

bool ChunkList::atBit(const int value, const unsigned int position) {
    // position equals zero means rightmost digit
    return ((value & (1 << position)) >> position);
}

int ChunkList::ambientOccIndex(int coordinates) {
    int coordMask = 0b11111111;
    int Z = ((coordinates & coordMask) + 4) / 8;
    int Y = (((coordinates & (coordMask << 8)) >> 8) + 4) / 8;
    int X = (((coordinates & (coordMask << 16)) >> 16) + 4) / 8;
    return (X << 2) + (Y << 1) + Z;
}

void ChunkList::combineFace(int coordX, int coordY, int coordZ, int blockID, int threadID) {

    // this is the cull bitmap, 1 means the face is culled, 0 means the face is visible
    // the order is as follows:
    // front
    // back
    // right
    // left
    // top
    // bottom
    unsigned int cullMap =    ((blocks[blockAt(coordX, coordY, coordZ + 1, threadID)].isSolid | (blockAt(coordX, coordY, coordZ + 1, threadID) == blockID)) << 5)
                            + ((blocks[blockAt(coordX, coordY, coordZ - 1, threadID)].isSolid | (blockAt(coordX, coordY, coordZ - 1, threadID) == blockID)) << 4)
                            + ((blocks[blockAt(coordX + 1, coordY, coordZ, threadID)].isSolid | (blockAt(coordX + 1, coordY, coordZ, threadID) == blockID)) << 3)
                            + ((blocks[blockAt(coordX - 1, coordY, coordZ, threadID)].isSolid | (blockAt(coordX - 1, coordY, coordZ, threadID) == blockID)) << 2)
                            + ((blocks[blockAt(coordX, coordY + 1, coordZ, threadID)].isSolid | (blockAt(coordX, coordY + 1, coordZ, threadID) == blockID)) << 1)
                            +  (blocks[blockAt(coordX, coordY - 1, coordZ, threadID)].isSolid | (blockAt(coordX, coordY - 1, coordZ, threadID) == blockID));

    unsigned int ambientOccMap =      (blocks[blockAt(coordX + 1, coordY + 1, coordZ + 1, threadID)].castsAO << 26)
                                    + (blocks[blockAt(coordX + 1, coordY + 1, coordZ, threadID)].castsAO << 25)
                                    + (blocks[blockAt(coordX + 1, coordY + 1, coordZ - 1, threadID)].castsAO << 24)
                                    + (blocks[blockAt(coordX + 1, coordY, coordZ + 1, threadID)].castsAO << 23)
                                    + (0 << 22)      //4
                                    + (blocks[blockAt(coordX + 1, coordY, coordZ - 1, threadID)].castsAO << 21)
                                    + (blocks[blockAt(coordX + 1, coordY - 1, coordZ + 1, threadID)].castsAO << 20)
                                    + (blocks[blockAt(coordX + 1, coordY - 1, coordZ, threadID)].castsAO << 19)
                                    + (blocks[blockAt(coordX + 1, coordY - 1, coordZ - 1, threadID)].castsAO << 18)
                                    + (blocks[blockAt(coordX, coordY + 1, coordZ + 1, threadID)].castsAO << 17)
                                    + (0 << 16)      //10
                                    + (blocks[blockAt(coordX, coordY + 1, coordZ - 1, threadID)].castsAO << 15)
                                    + (0 << 14)
                                    + (0 << 13)
                                    + (0 << 12)
                                    + (blocks[blockAt(coordX, coordY - 1, coordZ + 1, threadID)].castsAO << 11)
                                    + (0 << 10)      //15
                                    + (blocks[blockAt(coordX, coordY - 1, coordZ - 1, threadID)].castsAO << 9)
                                    + (blocks[blockAt(coordX - 1, coordY + 1, coordZ + 1, threadID)].castsAO << 8)
                                    + (blocks[blockAt(coordX - 1, coordY + 1, coordZ, threadID)].castsAO << 7)
                                    + (blocks[blockAt(coordX - 1, coordY + 1, coordZ - 1, threadID)].castsAO << 6)
                                    + (blocks[blockAt(coordX - 1, coordY, coordZ + 1, threadID)].castsAO << 5)
                                    + (0 << 4)
                                    + (blocks[blockAt(coordX - 1, coordY, coordZ - 1, threadID)].castsAO << 3)
                                    + (blocks[blockAt(coordX - 1, coordY - 1, coordZ + 1, threadID)].castsAO << 2)
                                    + (blocks[blockAt(coordX - 1, coordY - 1, coordZ, threadID)].castsAO << 1)
                                    + (blocks[blockAt(coordX - 1, coordY - 1, coordZ - 1, threadID)].castsAO << 0);

    // the number refers to the coordinate of the vertex wrt the coordinate of the bottom left back vertex of the cube
    // these are the masks for vertexes of the cube
    unsigned int vert111 = 0b110100000100000000000000000;
    unsigned int vert110 = 0b011001000001000000000000000;
    unsigned int vert101 = 0b000100110000000100000000000;
    unsigned int vert100 = 0b000001011000000001000000000;
    unsigned int vert011 = 0b000000000100000000110100000;
    unsigned int vert010 = 0b000000000001000000011001000;
    unsigned int vert001 = 0b000000000000000100000100110;
    unsigned int vert000 = 0b000000000000000001000001011;

    vert111 &= ambientOccMap;
    vert110 &= ambientOccMap;
    vert101 &= ambientOccMap;
    vert100 &= ambientOccMap;
    vert011 &= ambientOccMap;
    vert010 &= ambientOccMap;
    vert001 &= ambientOccMap;
    vert000 &= ambientOccMap;


    // 0 means the vertex has light, 1 means its dark
    unsigned int ambientOcc =     ((std::popcount(vert111) >= 2) << 7)
                                + ((std::popcount(vert110) >= 2) << 6)
                                + ((std::popcount(vert101) >= 2) << 5)
                                + ((std::popcount(vert100) >= 2) << 4)
                                + ((std::popcount(vert011) >= 2) << 3)
                                + ((std::popcount(vert010) >= 2) << 2)
                                + ((std::popcount(vert001) >= 2) << 1)
                                + ((std::popcount(vert000) >= 2) << 0);

    unsigned int finalMap = (ambientOcc << 18/*8 bits*/) + (cullMap << 12/*6 bits*/) + (coordX << 8/*4 bits*/) + (coordY << 4/*4 bits*/) + (coordZ/*4 bits*/);
    unsigned int blockIDMap = blockID;


    // we will be having 16x16x16 chunks, but each block will be made in an 8x8x8 grid, the mesh of the block will be predeterminned

    // vertex format:
    // | 4 bits for texture U | 4 bits for texture V | 8 bits for X | 8 bits for Y | 8 bits for Z |
    // | 28 bits for texture coordinate | 3 bits for normal | 1 bit for ambient occlusion |

    int offset = ((coordX << 16) + (coordY << 8) + (coordZ)) * 8;

    // generating the mesh of the block using the given template

    for (int i = 0; i < solidBlock.faceType.size(); i++) {
        if (!atBit(cullMap, solidBlock.faceType.at(i))) {
            int j = 8 * i;
            std::vector<int> face = std::vector<int>(8);
            face.at(0) = solidBlock.blockBitMap.at(j + 0) + offset;   face.at(1) = solidBlock.blockBitMap.at(j + 1) + atBit(ambientOcc, ambientOccIndex(solidBlock.blockBitMap.at(j + 0))) + (blockID << 4);
            face.at(2) = solidBlock.blockBitMap.at(j + 2) + offset;   face.at(3) = solidBlock.blockBitMap.at(j + 3) + atBit(ambientOcc, ambientOccIndex(solidBlock.blockBitMap.at(j + 2))) + (blockID << 4);
            face.at(4) = solidBlock.blockBitMap.at(j + 4) + offset;   face.at(5) = solidBlock.blockBitMap.at(j + 5) + atBit(ambientOcc, ambientOccIndex(solidBlock.blockBitMap.at(j + 4))) + (blockID << 4);
            face.at(6) = solidBlock.blockBitMap.at(j + 6) + offset;   face.at(7) = solidBlock.blockBitMap.at(j + 7) + atBit(ambientOcc, ambientOccIndex(solidBlock.blockBitMap.at(j + 6))) + (blockID << 4);
            
            chunkWorldContainer[index[threadID]].mesh.insert(chunkWorldContainer[index[threadID]].mesh.end(), face.begin(), face.end());
            doIndices(threadID);
        }
    }
}

int ChunkList::buildChunk(int threadID) {

    discardChunk[threadID] = 0;

    for (int i = 0; i < CHUNK_SIZE + 2; i++) {
        for (int j = 0; j < CHUNK_SIZE + 2; j++) {
            for (int k = 0; k < CHUNK_SIZE + 2; k++) {
                cachedBlocks[threadID][(i * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + (j * (CHUNK_SIZE + 2)) + k] = -1;
                if (i > 0 && j > 0 && k > 0 && i < (CHUNK_SIZE + 1) && j < (CHUNK_SIZE + 1) && k < (CHUNK_SIZE + 1))
                    cachedBlocks[threadID][(i * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + (j * (CHUNK_SIZE + 2)) + k] = chunkWorldContainer[index[threadID]].chunkData[((i - 1) * CHUNK_SIZE * CHUNK_SIZE) + ((j - 1) * CHUNK_SIZE) + (k - 1)];
            }
        }
    }

    combineFace(0, 0, 0, chunkWorldContainer[index[threadID]].chunkData[0], threadID);
    combineFace(CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, chunkWorldContainer[index[threadID]].chunkData[(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) - 1], threadID);
    if (discardChunk[threadID] == 1) {
        chunkWorldContainer[index[threadID]].unCompiledChunk = 1;
        return 0;
    }

    chunkWorldContainer[index[threadID]].indices.clear();
    chunkWorldContainer[index[threadID]].mesh.clear();

    call[threadID] = 0;

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (chunkWorldContainer[index[threadID]].chunkData[(i * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + k] > 0) {
                    combineFace(i, j, k, chunkWorldContainer[index[threadID]].chunkData[(i * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + k], threadID);
                }
                if (discardChunk[threadID] == 1) {
                    chunkWorldContainer[index[threadID]].unCompiledChunk = 1;
                    return 0;
                }
            }
        }
    }
    

    if (discardChunk[threadID] == 1) {
        chunkWorldContainer[index[threadID]].unCompiledChunk = 1;
        return 0;
    } else {
        chunkWorldContainer[index[threadID]].unCompiledChunk = 0;
        chunkWorldContainer[index[threadID]].forUpdate = 0;
    }
    
    return chunkWorldContainer[index[threadID]].indices.size();
}