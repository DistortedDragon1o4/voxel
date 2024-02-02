#include "../include/chunkList.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "fastFloat.h"
#include <thread>
#include <vector>

long WorldContainer::coordsToKey(const ChunkCoords coords) {
    long result = (fastFloat::mod(coords.x, 2 * RENDER_DISTANCE) * 4 * RENDER_DISTANCE * RENDER_DISTANCE) + (fastFloat::mod(coords.y, 2 * RENDER_DISTANCE) * 2 * RENDER_DISTANCE) + fastFloat::mod(coords.z, 2 * RENDER_DISTANCE);
    return result;
}

int WorldContainer::getIndex(const ChunkCoords chunkCoord) {
    int index;
    try {
        index = coordToIndexMap.at(coordsToKey(chunkCoord));
    } catch (const std::out_of_range& oor) {
        //std::cerr << "Out of Range error: " << oor.what() << "\n";
        return -1;
    }
    return index;
}

int WorldContainer::getIndex(const int chunkCoordX, const int chunkCoordY, const int chunkCoordZ) {
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

int ChunkBuilder::globalBlockAt(int coordX, int coordY, int coordZ, int index) {
    ChunkCoords currentChunk;
    currentChunk.x = chunkX;
    currentChunk.y = chunkY;
    currentChunk.z = chunkZ;
    int x = coordX;
    int y = coordY;
    int z = coordZ;
    if (coordX < 0) {
        currentChunk.x = chunkX - 1;
        x = CHUNK_SIZE - 1;
    } else if (coordX >= CHUNK_SIZE) {
        currentChunk.x = chunkX + 1;
        x = 0;
    }
    if (coordY < 0) {
        currentChunk.y = chunkY - 1;
        y = CHUNK_SIZE - 1;
    } else if (coordY >= CHUNK_SIZE) {
        currentChunk.y = chunkY + 1;
        y = 0;
    }
    if (coordZ < 0) {
        currentChunk.z = chunkZ - 1;
        z = CHUNK_SIZE - 1;
    } else if (coordZ >= CHUNK_SIZE) {
        currentChunk.z = chunkZ + 1;
        z = 0;
    }

    bool match = 0;
    int currentIndex = worldContainer.getIndex(currentChunk);

    if (currentIndex >= 0 && currentIndex < worldContainer.chunks.size() && worldContainer.chunks.at(currentIndex).chunkData.size() == CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) {
        try {
            return worldContainer.chunks.at(currentIndex).chunkData.at((x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + z);
        } catch (const std::out_of_range& oor) {
            std::cerr << "Out of Range error: " << oor.what() << "\n";
            discardChunk = 1;
            return 0;
        }
    } else {
        discardChunk = 1;
    }
    return 0;
}

int ChunkBuilder::cachedBlockAt(int coordX, int coordY, int coordZ) {
    return cachedBlocks[(coordX * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + (coordY * (CHUNK_SIZE + 2)) + coordZ];
}

int ChunkBuilder::blockAt(int coordX, int coordY, int coordZ, int index) {
    int block = cachedBlockAt(coordX + 1, coordY + 1, coordZ + 1);
    if (block == -1) {
        block = globalBlockAt(coordX, coordY, coordZ, index);
        cachedBlocks[((coordX + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((coordY + 1) * (CHUNK_SIZE + 2)) + (coordZ + 1)] = block;
    }
    return block;
}

// bool ChunkList::atBit(const int value, const unsigned int position) {
//     // position equals zero means rightmost digit
//     return ((value & (1 << position)) >> position);
// }

int ChunkBuilder::ambientOccIndex(int coordinates) {
    int coordMask = 0b1111111111;
    int Z = ((coordinates & coordMask) + 8) >> 4;
    int Y = (((coordinates >> 10) & coordMask) + 8) >> 4;
    int X = (((coordinates >> 20) & coordMask) + 8) >> 4;
    return ((X << 2) + (Y << 1) + Z) << 1;
}

void ChunkBuilder::combineFace(int coordX, int coordY, int coordZ, int blockID, int index) {
    Blocks &crntBlock = blocks.blocks[blockID];

    // this is the cull bitmap, 1 means the face is culled, 0 means the face is visible
    // the order is as follows:
    // front
    // back
    // right
    // left
    // top
    // bottom
    unsigned int cullMap =    ((blocks.blocks[blockAt(coordX, coordY, coordZ + 1, index)].isSolid | (blockAt(coordX, coordY, coordZ + 1, index) == blockID)) << 5)
                            + ((blocks.blocks[blockAt(coordX, coordY, coordZ - 1, index)].isSolid | (blockAt(coordX, coordY, coordZ - 1, index) == blockID)) << 4)
                            + ((blocks.blocks[blockAt(coordX + 1, coordY, coordZ, index)].isSolid | (blockAt(coordX + 1, coordY, coordZ, index) == blockID)) << 3)
                            + ((blocks.blocks[blockAt(coordX - 1, coordY, coordZ, index)].isSolid | (blockAt(coordX - 1, coordY, coordZ, index) == blockID)) << 2)
                            + ((blocks.blocks[blockAt(coordX, coordY + 1, coordZ, index)].isSolid | (blockAt(coordX, coordY + 1, coordZ, index) == blockID)) << 1)
                            +  (blocks.blocks[blockAt(coordX, coordY - 1, coordZ, index)].isSolid | (blockAt(coordX, coordY - 1, coordZ, index) == blockID));

    unsigned int ambientOccMap =      (blocks.blocks[blockAt(coordX + 1, coordY + 1, coordZ + 1, index)].castsAO << 26)
                                    + (blocks.blocks[blockAt(coordX + 1, coordY + 1, coordZ, index)].castsAO << 25)
                                    + (blocks.blocks[blockAt(coordX + 1, coordY + 1, coordZ - 1, index)].castsAO << 24)
                                    + (blocks.blocks[blockAt(coordX + 1, coordY, coordZ + 1, index)].castsAO << 23)
                                    + (0 << 22)      //4
                                    + (blocks.blocks[blockAt(coordX + 1, coordY, coordZ - 1, index)].castsAO << 21)
                                    + (blocks.blocks[blockAt(coordX + 1, coordY - 1, coordZ + 1, index)].castsAO << 20)
                                    + (blocks.blocks[blockAt(coordX + 1, coordY - 1, coordZ, index)].castsAO << 19)
                                    + (blocks.blocks[blockAt(coordX + 1, coordY - 1, coordZ - 1, index)].castsAO << 18)
                                    + (blocks.blocks[blockAt(coordX, coordY + 1, coordZ + 1, index)].castsAO << 17)
                                    + (0 << 16)      //10
                                    + (blocks.blocks[blockAt(coordX, coordY + 1, coordZ - 1, index)].castsAO << 15)
                                    + (0 << 14)
                                    + (0 << 13)
                                    + (0 << 12)
                                    + (blocks.blocks[blockAt(coordX, coordY - 1, coordZ + 1, index)].castsAO << 11)
                                    + (0 << 10)      //15
                                    + (blocks.blocks[blockAt(coordX, coordY - 1, coordZ - 1, index)].castsAO << 9)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY + 1, coordZ + 1, index)].castsAO << 8)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY + 1, coordZ, index)].castsAO << 7)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY + 1, coordZ - 1, index)].castsAO << 6)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY, coordZ + 1, index)].castsAO << 5)
                                    + (0 << 4)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY, coordZ - 1, index)].castsAO << 3)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY - 1, coordZ + 1, index)].castsAO << 2)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY - 1, coordZ, index)].castsAO << 1)
                                    + (blocks.blocks[blockAt(coordX - 1, coordY - 1, coordZ - 1, index)].castsAO << 0);

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

    unsigned int faceVert111 = 0b101010;
    unsigned int faceVert110 = 0b011010;
    unsigned int faceVert101 = 0b101001;
    unsigned int faceVert100 = 0b011001;
    unsigned int faceVert011 = 0b100110;
    unsigned int faceVert010 = 0b010110;
    unsigned int faceVert001 = 0b010101;
    unsigned int faceVert000 = 0b100101;

    faceVert111 &= cullMap;
    faceVert110 &= cullMap;
    faceVert101 &= cullMap;
    faceVert100 &= cullMap;
    faceVert011 &= cullMap;
    faceVert010 &= cullMap;
    faceVert001 &= cullMap;
    faceVert000 &= cullMap;


    // 0 means the vertex has light, 1 means its dark
    // unsigned int ambientOcc =     ((std::popcount(vert111) >= 2) << 7)
    //                             + ((std::popcount(vert110) >= 2) << 6)
    //                             + ((std::popcount(vert101) >= 2) << 5)
    //                             + ((std::popcount(vert100) >= 2) << 4)
    //                             + ((std::popcount(vert011) >= 2) << 3)
    //                             + ((std::popcount(vert010) >= 2) << 2)
    //                             + ((std::popcount(vert001) >= 2) << 1)
    //                             + ((std::popcount(vert000) >= 2) << 0);

    unsigned int ambientOcc =     (((std::popcount(vert111) >> 1) + (std::popcount(vert111) == 3) + (std::popcount(faceVert111) >= 1 && std::popcount(vert111) == 2)) << 14)
                                + (((std::popcount(vert110) >> 1) + (std::popcount(vert110) == 3) + (std::popcount(faceVert110) >= 1 && std::popcount(vert110) == 2)) << 12)
                                + (((std::popcount(vert101) >> 1) + (std::popcount(vert101) == 3) + (std::popcount(faceVert101) >= 1 && std::popcount(vert101) == 2)) << 10)
                                + (((std::popcount(vert100) >> 1) + (std::popcount(vert100) == 3) + (std::popcount(faceVert100) >= 1 && std::popcount(vert100) == 2)) << 8)
                                + (((std::popcount(vert011) >> 1) + (std::popcount(vert011) == 3) + (std::popcount(faceVert011) >= 1 && std::popcount(vert011) == 2)) << 6)
                                + (((std::popcount(vert010) >> 1) + (std::popcount(vert010) == 3) + (std::popcount(faceVert010) >= 1 && std::popcount(vert010) == 2)) << 4)
                                + (((std::popcount(vert001) >> 1) + (std::popcount(vert001) == 3) + (std::popcount(faceVert001) >= 1 && std::popcount(vert001) == 2)) << 2)
                                + (((std::popcount(vert000) >> 1) + (std::popcount(vert000) == 3) + (std::popcount(faceVert000) >= 1 && std::popcount(vert000) == 2)) << 0);

    unsigned int finalMap = (ambientOcc << 18/*8 bits*/) + (cullMap << 12/*6 bits*/) + (coordX << 8/*4 bits*/) + (coordY << 4/*4 bits*/) + (coordZ/*4 bits*/);

    // we will be having 32x32x32 chunks, but each block will be made in an 16x16x16 grid, the mesh of the block will be predeterminned

    // vertex format:
    // | 10 bits for X | 10 bits for Y | 10 bits for Z |
    // | 13 bits for texture coordinate | 4 bits for texture U | 4 bits for texture V | 3 bits for normal | 8 bit for ambient occlusion |

    int offset = ((coordX << 20) + (coordY << 10) + (coordZ)) << 4;

    // generating the mesh of the block using the given template

    for (int i = 0; i < crntBlock.model.size() / 36; i++) {
        if (!fastFloat::atBit(cullMap, crntBlock.faceType[i])) {
            int j = 8 * i;
            int ambientOccOfFace = (fastFloat::atBits(ambientOcc, ambientOccIndex(crntBlock.blockBitMap.at(j + 0)), 2) << 6) + (fastFloat::atBits(ambientOcc, ambientOccIndex(crntBlock.blockBitMap.at(j + 2)), 2) << 4) + (fastFloat::atBits(ambientOcc, ambientOccIndex(crntBlock.blockBitMap.at(j + 4)), 2) << 2) + fastFloat::atBits(ambientOcc, ambientOccIndex(crntBlock.blockBitMap.at(j + 6)), 2);
            std::vector<int> face = std::vector<int>(8);
            face.at(0) = crntBlock.blockBitMap.at(j + 0) + offset;   face.at(1) = crntBlock.blockBitMap.at(j + 1) + ambientOccOfFace;
            face.at(2) = crntBlock.blockBitMap.at(j + 2) + offset;   face.at(3) = crntBlock.blockBitMap.at(j + 3) + ambientOccOfFace;
            face.at(4) = crntBlock.blockBitMap.at(j + 4) + offset;   face.at(5) = crntBlock.blockBitMap.at(j + 5) + ambientOccOfFace;
            face.at(6) = crntBlock.blockBitMap.at(j + 6) + offset;   face.at(7) = crntBlock.blockBitMap.at(j + 7) + ambientOccOfFace;
            
            worldContainer.chunks[index].mesh.insert(worldContainer.chunks[index].mesh.end(), face.begin(), face.end());
        }
    }
}

void ChunkBuilder::buildChunk(int index) {

    discardChunk = 0;

    for (int i = 0; i < CHUNK_SIZE + 2; i++) {
        for (int j = 0; j < CHUNK_SIZE + 2; j++) {
            for (int k = 0; k < CHUNK_SIZE + 2; k++) {
                cachedBlocks[(i * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + (j * (CHUNK_SIZE + 2)) + k] = -1;
                if (i > 0 && j > 0 && k > 0 && i < (CHUNK_SIZE + 1) && j < (CHUNK_SIZE + 1) && k < (CHUNK_SIZE + 1))
                    cachedBlocks[(i * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + (j * (CHUNK_SIZE + 2)) + k] = worldContainer.chunks[index].chunkData[((i - 1) * CHUNK_SIZE * CHUNK_SIZE) + ((j - 1) * CHUNK_SIZE) + (k - 1)];
            }
        }
    }

    combineFace(0, 0, 0, worldContainer.chunks[index].chunkData[0], index);
    combineFace(CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, worldContainer.chunks[index].chunkData[(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) - 1], index);
    if (discardChunk == 1) {
        worldContainer.chunks[index].unCompiledChunk = 1;
        worldContainer.chunks[index].meshSize = 0;
        return;
    }

    worldContainer.chunks[index].mesh.clear();

    call = 0;

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (worldContainer.chunks[index].chunkData[(i * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + k] > 0) {
                    combineFace(i, j, k, worldContainer.chunks[index].chunkData[(i * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + k], index);
                }
                if (discardChunk == 1) {
                    worldContainer.chunks[index].unCompiledChunk = 1;
                    worldContainer.chunks[index].meshSize = 0;
                    return;
                }
            }
        }
    }
    

    if (discardChunk == 1) {
        worldContainer.chunks[index].unCompiledChunk = 1;
        worldContainer.chunks[index].meshSize = 0;
        return;
    } else {
        worldContainer.chunks[index].unCompiledChunk = 0;
        worldContainer.chunks[index].forUpdate = 0;
    }
    worldContainer.chunks[index].meshSize = worldContainer.chunks[index].mesh.size();
    worldContainer.chunks[index].redoRegionMesh = true;
}