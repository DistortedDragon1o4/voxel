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

int ChunkBuilder::blockAt(int coordX, int coordY, int coordZ, ChunkDataContainer &crntChunk) {
    int a = 1;
    int b = 1;
    int c = 1;

    if (coordX < 0) {
        a = 0;
    } else if (coordX >= CHUNK_SIZE) {
        a = 2;
    }
    if (coordY < 0) {
        b = 0;
    } else if (coordY >= CHUNK_SIZE) {
        b = 2;
    }
    if (coordZ < 0) {
        c = 0;
    } else if (coordZ >= CHUNK_SIZE) {
        c = 2;
    }

    int neighbourIndex = crntChunk.neighbouringChunkIndices[(a * 9) + (b * 3) + c];
    if (neighbourIndex != -1 && worldContainer.chunks[neighbourIndex].unGeneratedChunk == false) {
        return worldContainer.chunks[neighbourIndex].blockAtCoords(fastFloat::mod(coordX, CHUNK_SIZE), fastFloat::mod(coordY, CHUNK_SIZE), fastFloat::mod(coordZ, CHUNK_SIZE));
    } else {
        discardChunk = true;
        return 0;
    }
}

int ChunkBuilder::ambientOccIndex(int coordinates) {
    int coordMask = 0b1111111111;
    int Z = ((coordinates & coordMask) + 8) >> 4;
    int Y = (((coordinates >> 10) & coordMask) + 8) >> 4;
    int X = (((coordinates >> 20) & coordMask) + 8) >> 4;
    return ((X << 2) + (Y << 1) + Z) << 1;
}

void ChunkBuilder::combineFace(int coordX, int coordY, int coordZ, ChunkDataContainer &chunk) {
    int blockID = chunk.blockAtCoords(coordX, coordY, coordZ);
    Blocks &crntBlock = blocks.blocks[blockID];

    // this is the cull bitmap, 1 means the face is culled, 0 means the face is visible
    // the order is as follows:
    // front
    // back
    // right
    // left
    // top
    // bottom
    unsigned int cullMap =    ((blocks.blocks[blockAt(coordX, coordY, coordZ + 1, chunk)].isSolid | (blockAt(coordX, coordY, coordZ + 1, chunk) == blockID)) << 5)
                            + ((blocks.blocks[blockAt(coordX, coordY, coordZ - 1, chunk)].isSolid | (blockAt(coordX, coordY, coordZ - 1, chunk) == blockID)) << 4)
                            + ((blocks.blocks[blockAt(coordX + 1, coordY, coordZ, chunk)].isSolid | (blockAt(coordX + 1, coordY, coordZ, chunk) == blockID)) << 3)
                            + ((blocks.blocks[blockAt(coordX - 1, coordY, coordZ, chunk)].isSolid | (blockAt(coordX - 1, coordY, coordZ, chunk) == blockID)) << 2)
                            + ((blocks.blocks[blockAt(coordX, coordY + 1, coordZ, chunk)].isSolid | (blockAt(coordX, coordY + 1, coordZ, chunk) == blockID)) << 1)
                            +  (blocks.blocks[blockAt(coordX, coordY - 1, coordZ, chunk)].isSolid | (blockAt(coordX, coordY - 1, coordZ, chunk) == blockID));

    unsigned int prevCullMap =    ((blocks.blocks[blockAt(coordX + 1, coordY, coordZ - 1, chunk)].isSolid | (blockAt(coordX + 1, coordY, coordZ, chunk) == blockID)) << 3)
                                + ((blocks.blocks[blockAt(coordX - 1, coordY, coordZ - 1, chunk)].isSolid | (blockAt(coordX - 1, coordY, coordZ, chunk) == blockID)) << 2)
                                + ((blocks.blocks[blockAt(coordX, coordY + 1, coordZ - 1, chunk)].isSolid | (blockAt(coordX, coordY + 1, coordZ, chunk) == blockID)) << 1)
                                +  (blocks.blocks[blockAt(coordX, coordY - 1, coordZ - 1, chunk)].isSolid | (blockAt(coordX, coordY - 1, coordZ, chunk) == blockID));

    int previousBlockID = blockAt(coordX, coordY, coordZ - 1, chunk);
    if (coordZ == 0)
        previousBlockID = 0;

    // we will be having 32x32x32 chunks, but each block will be made in an 16x16x16 grid, the mesh of the block will be predeterminned

    // vertex format:
    // | 10 bits for X | 10 bits for Y | 10 bits for Z |
    // | 20 bits for texture coordinate | 4 bits for texture U | 4 bits for texture V | 3 bits for normal | 1 bit for ambient occlusion |

    int offset = ((coordX << 20) + (coordY << 10) + (coordZ)) << 4;

    // generating the mesh of the block using the given template

    for (int i = 0; i < crntBlock.model.size() / 36; i++) {
        if (!fastFloat::atBit(cullMap, crntBlock.faceType[i])) {
            int faceType = (crntBlock.blockBitMap.at((8 * i) + 1) >> 1) & 0b111;

            if ((temporaryMesh.at(faceType).size() == 0) || (previousBlockID != blockID) || (faceType > 3) || (fastFloat::atBit(prevCullMap, crntBlock.faceType[i])) || crntBlock.faceType[i] >= 6) {
                int j = 8 * i;
                std::vector<int> face = std::vector<int>(8);
                face.at(0) = crntBlock.blockBitMap.at(j + 0) + offset;   face.at(1) = crntBlock.blockBitMap.at(j + 1);
                face.at(2) = crntBlock.blockBitMap.at(j + 2) + offset;   face.at(3) = crntBlock.blockBitMap.at(j + 3);
                face.at(4) = crntBlock.blockBitMap.at(j + 4) + offset;   face.at(5) = crntBlock.blockBitMap.at(j + 5);
                face.at(6) = crntBlock.blockBitMap.at(j + 6) + offset;   face.at(7) = crntBlock.blockBitMap.at(j + 7);
                
                temporaryMesh.at(faceType).insert(temporaryMesh.at(faceType).end(), face.begin(), face.end());
            } else {
                for (int j = 0; j < 4; j++) {
                    // std:: cout << temporaryMesh.at(faceType).size() << " " << j << " " << temporaryMesh.at(faceType).size() - 8 + (2 * j) << "\n";
                    if (((crntBlock.blockBitMap.at((8 * i) + (2 * j)) & 0b11111) == 0b10000))
                        temporaryMesh.at(faceType).at(temporaryMesh.at(faceType).size() - 8 + (2 * j)) += 0b10000;
                }
            }
        }
    }
}

int ChunkBuilder::buildChunk(ChunkDataContainer &chunk) {

    discardChunk = false;

    for (int i = 0; i < 6; i++)
        temporaryMesh[i].clear();

    // Early abort in case chunk is filled with air
    if (chunk.isSingleBlock && chunk.theBlock == 0) {
        chunk.unCompiledChunk = 0;
        chunk.forUpdate = 0;
        chunk.meshSize = 0;
        chunk.reUploadMesh = true;
        return 0;
    }

    // Early abort in case of unfilled neighbouring chunks
    for (int i : chunk.neighbouringChunkIndices) {
        if (i == -1) {
            chunk.unCompiledChunk = 1;
            chunk.meshSize = 0;
            return -1;
        } else {
            if (worldContainer.chunks[i].unGeneratedChunk == true) {
                chunk.unCompiledChunk = 1;
                chunk.meshSize = 0;
                return -1;
            }
        }
    }

    chunk.mesh.clear();

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (chunk.blockAtCoords(i, j, k) > 0) {
                    combineFace(i, j, k, chunk);
                }
                // Abort mechanism in case a chunk gets unloaded mid-way
                if (discardChunk == 1) {
                    chunk.unCompiledChunk = 1;
                    chunk.meshSize = 0;
                    return -1;
                }
            }
        }
    }
    

    if (discardChunk == 1) {
        chunk.unCompiledChunk = 1;
        chunk.meshSize = 0;
        return -1;
    }

    for (int i = 0; i < 6; i++)
        chunk.mesh.insert(chunk.mesh.end(), temporaryMesh[i].begin(), temporaryMesh[i].end());

    chunk.unCompiledChunk = 0;
    chunk.forUpdate = 0;
    chunk.meshSize = chunk.mesh.size();
    chunk.reUploadMesh = true;

    return 0;
}