#include "chunkDataContainer.h"
#include "chunkList.h"
#include "glm/fwd.hpp"

// Frustum culling

bool FrustumCuller::isFrustumCulled(const ChunkCoords &chunkCoords) {
	glm::dvec3 chunkBaseCoords = glm::dvec3(chunkCoords.x * CHUNK_SIZE, chunkCoords.y * CHUNK_SIZE, chunkCoords.z * CHUNK_SIZE);
	glm::dvec3 camPos = glm::dvec3(camera.Position.x, camera.Position.y, camera.Position.z) - (frustumOffset * camera.Orientation);
	for (int i = 0; i < CHUNK_SIZE + 1; i += CHUNK_SIZE) {
		for (int j = 0; j < CHUNK_SIZE + 1; j += CHUNK_SIZE) {
			for (int k = 0; k < CHUNK_SIZE + 1; k += CHUNK_SIZE) {
				glm::dvec3 crntVertCoords = glm::dvec3(chunkBaseCoords.x + i, chunkBaseCoords.y + j, chunkBaseCoords.z + k);
				double dot = glm::dot(glm::normalize(crntVertCoords - camPos), camera.Orientation);
				if (dot >= cosineModifiedHalfFOV)
					return true;
			}
		}
	}
	return false;
}


// Check the permeability of a chunk for occlusion culling

int ChunkPermeability::getBlockIndex(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE)
        return -1;
    if (y < 0 || y >= CHUNK_SIZE)
        return -1;
    if (z < 0 || z >= CHUNK_SIZE)
        return -1;
    return (x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + z;
}

short ChunkPermeability::facing(int index) {
    short facingInfo = 0;
    BlockCoords block(index);

    if (block.x == CHUNK_SIZE - 1)
        facingInfo |= (0b1 << 0);
    if (block.x == 0)
        facingInfo |= (0b1 << 1);

    if (block.y == CHUNK_SIZE - 1)
        facingInfo |= (0b1 << 2);
    if (block.y == 0)
        facingInfo |= (0b1 << 3);

    if (block.z == CHUNK_SIZE - 1)
        facingInfo |= (0b1 << 4);
    if (block.z == 0)
        facingInfo |= (0b1 << 5);

    return facingInfo;
}

void ChunkPermeability::searchNeighbouringBlocks(int blockIndex, ChunkDataContainer &chunk, std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> &chunkDataBFSvisited) {
    BlockCoords block(blockIndex);
    std::array<int, 6> neighbouringBlockIndices;

    neighbouringBlockIndices.at(0) = getBlockIndex(block.x + 1, block.y, block.z);

    neighbouringBlockIndices.at(1) = getBlockIndex(block.x - 1, block.y, block.z);

    neighbouringBlockIndices.at(2) = getBlockIndex(block.x, block.y, block.z + 1);

    neighbouringBlockIndices.at(3) = getBlockIndex(block.x, block.y, block.z - 1);

    neighbouringBlockIndices.at(4) = getBlockIndex(block.x, block.y + 1, block.z);

    neighbouringBlockIndices.at(5) = getBlockIndex(block.x, block.y - 1, block.z);

    for (int i = 0; i < 6; i++) {
        int index = neighbouringBlockIndices[i];
        if (index != -1) {
            if (chunkDataBFSvisited[index] == false && blocks.blocks[chunk.chunkData[index]].isSolid == false) {
                BFSqueuePermeability.push(index);
                chunkDataBFSvisited[index] = true;
            }
        }
    }
}

void ChunkPermeability::doBlockBFSforPermeability(int startIndex, short &neighbours, ChunkDataContainer &chunk, std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> &chunkDataBFSvisited) {
    int crntBlockIndex = startIndex;
    chunkDataBFSvisited[crntBlockIndex] = true;
    neighbours |= facing(crntBlockIndex);
    searchNeighbouringBlocks(crntBlockIndex, chunk, chunkDataBFSvisited);
    while(BFSqueuePermeability.size() > 0) {
        crntBlockIndex = BFSqueuePermeability.front();
        BFSqueuePermeability.pop();
        neighbours |= facing(crntBlockIndex);
        searchNeighbouringBlocks(crntBlockIndex, chunk, chunkDataBFSvisited);
    }
}

void ChunkPermeability::checkPermeability(ChunkDataContainer &chunk) {
    std::array<bool, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> chunkDataBFSvisited = {0};
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
        if (chunkDataBFSvisited[i] == false && blocks.blocks[chunk.chunkData[i]].isSolid == false) {
            Region crntRegion;
            short neighbours;
            doBlockBFSforPermeability(i, neighbours, chunk, chunkDataBFSvisited);
            chunk.permeability |= generatePermeability(neighbours);
        }
    }
    chunk.isPermeableCheckDone = true;
}

short ChunkPermeability::generatePermeability(short &neighbours) {
    short permeability = 0;
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < i; j++)
            permeability |= ((fastFloat::atBit(neighbours, i) & fastFloat::atBit(neighbours, j)) << permeabilityIndex(i, j));
    return permeability;
}

short ChunkPermeability::permeabilityIndex(int a, int b) {
    int max = std::max(a, b);
    int min = std::min(a, b);

    return (((6 + min - max) * (7 + min - max)) / 2) - min - 1;
}


// BFS for making the meshing queue and also occlusion culling

void ChunkMeshingQueueGenerator::doBFS(const ChunkCoords chunk) {
    BFSqueue.empty();
    chunkMeshingQueue[!crntMeshingQueue].empty();

    ChunkCoords crntChunk;
    bool first = true;
    do {
        if (first) {
            crntChunk = chunk;
            first = false;
        } else {
            crntChunk = BFSqueue.front();
            BFSqueue.pop();
        }
        int crntChunkIndex = worldContainer.getIndex(crntChunk);
        if (crntChunkIndex == -1) {
            std::cout << "I died\n";
            return;
        }
        ChunkDataContainer &chunkObj = worldContainer.chunks.at(crntChunkIndex);
        if (chunkObj.unGeneratedChunk == false && chunkObj.isPermeableCheckDone == false)
            permeability.checkPermeability(chunkObj);
        if (chunkObj.unCompiledChunk == true && chunkObj.frustumVisible == true && chunkObj.unGeneratedChunk == false && chunkObj.isPermeableCheckDone == true && chunkObj.inQueue == false && worldContainer.isEdgeChunk(crntChunk.x, crntChunk.y, crntChunk.z) == false) {
            chunkMeshingQueue[!crntMeshingQueue].push(crntChunkIndex);
            chunkObj.inQueue = true;
        }
        localOcclusionUnCulled.at(crntChunkIndex) = true;
        if (chunkObj.unGeneratedChunk == false && chunkObj.isPermeableCheckDone == true)
            searchNeighbouringChunks(crntChunk);
    } while (BFSqueue.size() > 0);

    crntMeshingQueue = !crntMeshingQueue;
}

void ChunkMeshingQueueGenerator::searchNeighbouringChunks(const ChunkCoords chunkID) {
    int crntChunkIndex = worldContainer.getIndex(chunkID);
    ChunkDataContainer &crntChunk = worldContainer.chunks.at(crntChunkIndex);

    std::array<unsigned int, 6> neighbouringChunkIndices;

    // const auto start = std::chrono::high_resolution_clock::now();
    neighbouringChunkIndices.at(0) = worldContainer.getIndex(chunkID.x + 1, chunkID.y, chunkID.z);

    neighbouringChunkIndices.at(1) = worldContainer.getIndex(chunkID.x - 1, chunkID.y, chunkID.z);

    neighbouringChunkIndices.at(4) = worldContainer.getIndex(chunkID.x, chunkID.y, chunkID.z + 1);

    neighbouringChunkIndices.at(5) = worldContainer.getIndex(chunkID.x, chunkID.y, chunkID.z - 1);

    neighbouringChunkIndices.at(2) = worldContainer.getIndex(chunkID.x, chunkID.y + 1, chunkID.z);

    neighbouringChunkIndices.at(3) = worldContainer.getIndex(chunkID.x, chunkID.y - 1, chunkID.z);
    // const auto end = std::chrono::high_resolution_clock::now();
 
    // const std::chrono::duration<double> diff = end - start;
    // std::cout << "6 hashmap queries: " << diff << "\n";

    short backwardChunkIndices = 0;

    for (int i = 0; i < 6; i++) {
        if (neighbouringChunkIndices.at(i) != -1) {
            backwardChunkIndices |= (worldContainer.chunks.at(neighbouringChunkIndices.at(i)).distance < crntChunk.distance) << i;
        }
    }
    for (int i = 0; i < 6; i++) {
        if (neighbouringChunkIndices.at(i) != -1) {
            ChunkDataContainer &chunk = worldContainer.chunks.at(neighbouringChunkIndices.at(i));
            if (chunk.inBFSqueue == false && fastFloat::atBit(backwardChunkIndices, i) == 0) {
                bool willAddToQueue = false;
                for (int j = 0; j < 6; j++)
                    if (j != i)
                        if (fastFloat::atBit(backwardChunkIndices, j))
                            if (fastFloat::atBit(crntChunk.permeability, permeability.permeabilityIndex(i, j)))
                                willAddToQueue = true;
                if (willAddToQueue || backwardChunkIndices == 0) {
                    BFSqueue.push(chunk.chunkID);
                    chunk.inBFSqueue = true;
                }
            }
        }
    }
}



// BFS for making the generating queue

void ChunkGeneratingQueueGenerator::doBFS(const ChunkCoords chunk) {
    BFSqueue.empty();
    chunkGeneratingQueue[!crntGeneratingQueue].empty();

    ChunkCoords crntChunk;
    bool first = true;
    do {
        if (first) {
            crntChunk = chunk;
            first = false;
        } else {
            crntChunk = BFSqueue.front();
            BFSqueue.pop();
        }
        int crntChunkIndex = worldContainer.getIndex(crntChunk);
        if (crntChunkIndex == -1) {
            std::cout << "I died\n";
            return;
        }
        ChunkDataContainer &chunkObj = worldContainer.chunks.at(crntChunkIndex);
        if (chunkObj.unGeneratedChunk == true && chunkObj.inGeneratorQueue == false) {
            chunkGeneratingQueue[!crntGeneratingQueue].push(crntChunkIndex);
            chunkObj.inGeneratorQueue = true;
        }
        if (chunkObj.unGeneratedChunk == true)
            searchNeighbouringChunks(crntChunk);
    } while (BFSqueue.size() > 0);

    crntGeneratingQueue = !crntGeneratingQueue;
}

void ChunkGeneratingQueueGenerator::searchNeighbouringChunks(const ChunkCoords chunkID) {
    std::array<unsigned int, 6> neighbouringChunkIndices;

    neighbouringChunkIndices.at(0) = worldContainer.getIndex(chunkID.x + 1, chunkID.y, chunkID.z);

    neighbouringChunkIndices.at(1) = worldContainer.getIndex(chunkID.x - 1, chunkID.y, chunkID.z);

    neighbouringChunkIndices.at(4) = worldContainer.getIndex(chunkID.x, chunkID.y, chunkID.z + 1);

    neighbouringChunkIndices.at(5) = worldContainer.getIndex(chunkID.x, chunkID.y, chunkID.z - 1);

    neighbouringChunkIndices.at(2) = worldContainer.getIndex(chunkID.x, chunkID.y + 1, chunkID.z);

    neighbouringChunkIndices.at(3) = worldContainer.getIndex(chunkID.x, chunkID.y - 1, chunkID.z);

    for (int i = 0; i < 6; i++) {
        if (neighbouringChunkIndices.at(i) != -1) {
            ChunkDataContainer &chunk = worldContainer.chunks.at(neighbouringChunkIndices.at(i));
            if (chunk.inGenBFSqueue == false) {
                BFSqueue.push(chunk.chunkID);
                chunk.inGenBFSqueue = true;
            }
        }
    }
}