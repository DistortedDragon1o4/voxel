#include "chunkList.h"
#include "glm/fwd.hpp"

// Frustum culling

bool ChunkList::isFrustumCulled(ChunkDataContainer &chunk) {
	glm::dvec3 chunkBaseCoords = glm::dvec3(chunk.chunkID.at(0) * CHUNK_SIZE, chunk.chunkID.at(1) * CHUNK_SIZE, chunk.chunkID.at(2) * CHUNK_SIZE);
	glm::dvec3 camPos = glm::dvec3(camPosX, camPosY, camPosZ) - (frustumOffset * camDir);
	for (int i = 0; i < CHUNK_SIZE + 1; i += CHUNK_SIZE) {
		for (int j = 0; j < CHUNK_SIZE + 1; j += CHUNK_SIZE) {
			for (int k = 0; k < CHUNK_SIZE + 1; k += CHUNK_SIZE) {
				glm::dvec3 crntVertCoords = glm::dvec3(chunkBaseCoords.x + i, chunkBaseCoords.y + j, chunkBaseCoords.z + k);
				double dot = glm::dot(glm::normalize(crntVertCoords - camPos), camDir);
				if (dot >= cosineModifiedHalfFOV)
					return true;
			}
		}
	}
	return false;
}


// Check the permeability of a chunk for occlusion culling

int ChunkList::getBlockIndex(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE)
        return -1;
    if (y < 0 || y >= CHUNK_SIZE)
        return -1;
    if (z < 0 || z >= CHUNK_SIZE)
        return -1;
    return (x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + z;
}

short ChunkList::facing(int index) {
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

void ChunkList::searchNeighbouringBlocks(Region &region, int blockIndex, ChunkDataContainer &chunk) {
    BlockCoords block(blockIndex);
    std::array<int, 6> neighbouringBlockIndices;

    neighbouringBlockIndices.at(0) = getBlockIndex(block.x + 1, block.y, block.z);

    neighbouringBlockIndices.at(1) = getBlockIndex(block.x - 1, block.y, block.z);

    neighbouringBlockIndices.at(2) = getBlockIndex(block.x, block.y, block.z + 1);

    neighbouringBlockIndices.at(3) = getBlockIndex(block.x, block.y, block.z - 1);

    neighbouringBlockIndices.at(4) = getBlockIndex(block.x, block.y + 1, block.z);

    neighbouringBlockIndices.at(5) = getBlockIndex(block.x, block.y - 1, block.z);

    for (int i = 0; i < 6; i++) {
        if (neighbouringBlockIndices.at(i) != -1) {
            if (blocks.at(chunk.chunkData.at(neighbouringBlockIndices.at(i))).isSolid == false) {
                if (chunk.chunkDataBFSvisited.at(neighbouringBlockIndices.at(i)) == false) {
                    BFSqueuePermeability.push(neighbouringBlockIndices.at(i));
                    chunk.chunkDataBFSvisited.at(neighbouringBlockIndices.at(i)) = true;
                    //region.neighbours |= facing(neighbouringBlockIndices.at(i));
                }
            }
        }
    }
}

void ChunkList::doBlockBFSforPermeability(int startIndex, Region &region, ChunkDataContainer &chunk) {
    int crntBlockIndex = startIndex;
    chunk.chunkDataBFSvisited.at(crntBlockIndex) = true;
    region.neighbours |= facing(crntBlockIndex);
    searchNeighbouringBlocks(region, crntBlockIndex, chunk);
    while(BFSqueuePermeability.size() > 0) {
        crntBlockIndex = BFSqueuePermeability.front();
        BFSqueuePermeability.pop();
        region.neighbours |= facing(crntBlockIndex);
        searchNeighbouringBlocks(region, crntBlockIndex, chunk);
    }
}

void ChunkList::checkPermeability(ChunkDataContainer &chunk) {
    //std::vector<Region> regions;
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
        if (blocks.at(chunk.chunkData.at(i)).isSolid == false && chunk.chunkDataBFSvisited.at(i) == false) {
            Region crntRegion;
            doBlockBFSforPermeability(i, crntRegion, chunk);
            // std::bitset<16> x(crntRegion.neighbours);
            // std::cout << x << " happy\n";
            chunk.permeability |= generatePermeability(crntRegion);
        }
    }
    chunk.isPermeableCheckDone = true;
    // std::cout << "done\n";
}

short ChunkList::generatePermeability(Region &region) {
    short permeability = 0;
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < i; j++)
            permeability |= ((atBit(region.neighbours, i) & atBit(region.neighbours, j)) << permeabilityIndex(i, j));
    return permeability;
}

short ChunkList::permeabilityIndex(int a, int b) {
    int max = std::max(a, b);
    int min = std::min(a, b);

    return (((6 + min - max) * (7 + min - max)) / 2) - min - 1;
}


// BFS for making the meshing queue and also occlusion culling

void ChunkList::doBFS(std::array<int, 3> chunk) {

    std::array<int, 3> crntChunk;
    bool first = true;
    // int count = 0;
    // int x, y = 0;
    do {
        if (first) {
            crntChunk = chunk;
            first = false;
        } else {
            crntChunk = BFSqueue.front();
            BFSqueue.pop();
        }
        int crntChunkIndex = getIndex(crntChunk.at(0), crntChunk.at(1), crntChunk.at(2));
        if (crntChunkIndex == -1) {
            std::cout << "I died\n";
            return;
        }
        if (chunkWorldContainer.at(crntChunkIndex).unCompiledChunk == true && isFrustumCulled(chunkWorldContainer.at(crntChunkIndex)) && chunkWorldContainer.at(crntChunkIndex).unGeneratedChunk == false && chunkWorldContainer.at(crntChunkIndex).isPermeableCheckDone == true && chunkWorldContainer.at(crntChunkIndex).inQueue == false && isEdgeChunk(crntChunk.at(0), crntChunk.at(1), crntChunk.at(2)) == false) {
            chunkMeshingQueue.push(crntChunkIndex);
            chunkWorldContainer.at(crntChunkIndex).inQueue = true;
            // x++;
        }
        localOcclusionUnCulled.at(crntChunkIndex) = true;
        // y++;
        // std::cout << chunkMeshingQueue.size() << " " << x << " " << BFSqueue.size() << " " << crntChunk.at(0) << " " << crntChunk.at(1) << " " << crntChunk.at(2) << " Sad\n";
        if (chunkWorldContainer.at(crntChunkIndex).unGeneratedChunk == false && chunkWorldContainer.at(crntChunkIndex).isPermeableCheckDone == true)
            searchNeighbouringChunks(crntChunk);
    } while (BFSqueue.size() > 0);
    // std::cout << chunkMeshingQueue.size() << " " << x << " " << y << " " << crntChunk.at(0) << " " << crntChunk.at(1) << " " << crntChunk.at(2) << " Sad\n";
}

void ChunkList::searchNeighbouringChunks(std::array<int, 3> chunkID) {
    int crntChunkIndex = getIndex(chunkID.at(0), chunkID.at(1), chunkID.at(2));
    ChunkDataContainer &crntChunk = chunkWorldContainer.at(crntChunkIndex);

    std::array<unsigned int, 6> neighbouringChunkIndices;

    // const auto start = std::chrono::high_resolution_clock::now();
    neighbouringChunkIndices.at(0) = getIndex(chunkID.at(0) + 1, chunkID.at(1), chunkID.at(2));

    neighbouringChunkIndices.at(1) = getIndex(chunkID.at(0) - 1, chunkID.at(1), chunkID.at(2));

    neighbouringChunkIndices.at(4) = getIndex(chunkID.at(0), chunkID.at(1), chunkID.at(2) + 1);

    neighbouringChunkIndices.at(5) = getIndex(chunkID.at(0), chunkID.at(1), chunkID.at(2) - 1);

    neighbouringChunkIndices.at(2) = getIndex(chunkID.at(0), chunkID.at(1) + 1, chunkID.at(2));

    neighbouringChunkIndices.at(3) = getIndex(chunkID.at(0), chunkID.at(1) - 1, chunkID.at(2));
    // const auto end = std::chrono::high_resolution_clock::now();
 
    // const std::chrono::duration<double> diff = end - start;
    // std::cout << "6 hashmap queries: " << diff << "\n";

    short backwardChunkIndices = 0;

    for (int i = 0; i < 6; i++) {
        if (neighbouringChunkIndices.at(i) != -1) {
            backwardChunkIndices |= (chunkWorldContainer.at(neighbouringChunkIndices.at(i)).distance < crntChunk.distance) << i;
        }
    }
    for (int i = 0; i < 6; i++) {
        if (neighbouringChunkIndices.at(i) != -1) {
            ChunkDataContainer &chunk = chunkWorldContainer.at(neighbouringChunkIndices.at(i));
            if (chunk.inBFSqueue == false && chunk.isPermeableCheckDone == true && atBit(backwardChunkIndices, i) == 0) {
                bool willAddToQueue = false;
                for (int j = 0; j < 6; j++)
                    if (j != i)
                        if (atBit(backwardChunkIndices, j))
                            if (atBit(crntChunk.permeability, permeabilityIndex(i, j)))
                                willAddToQueue = true;
                if (willAddToQueue || backwardChunkIndices == 0) {
                    BFSqueue.push(chunk.chunkID);
                    chunk.inBFSqueue = true;
                }
            }
        }
    }
}