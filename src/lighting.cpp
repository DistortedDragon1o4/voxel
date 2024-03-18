#include "../include/chunkList.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "coordinateContainers.h"

ChunkLighting::ChunkLighting(WorldContainer &worldContainer, BlockDefs &blocks):
    worldContainer(worldContainer),
    blocks(blocks) {
        lightDataBuffer.create();
        lightDataBuffer.allocate(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE * sizeof(ChunkLightContainer), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        lightDataBuffer.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        lightDataBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
    }

void ChunkLighting::updateLight(ChunkDataContainer &chunk) {
    // Early abort in case of unfilled neighbouring chunks
    for (int i : chunk.neighbouringChunkIndices)
        if (i == -1)
            return;
        else
            if (worldContainer.chunks[i].unGeneratedChunk == true)
                return;

    bool lightDataWasModified = false;
    while(chunk.lightUpdateInstructions.size() > 0) {
        LightUpdateInstruction crntInstruction = chunk.lightUpdateInstructions.front();
        chunk.lightUpdateInstructions.erase(chunk.lightUpdateInstructions.begin());

        if (crntInstruction.propagationType) {
            propagateLight(crntInstruction.coords, crntInstruction.propagateChannel, crntInstruction.value, chunk);
        } else {
            depropagateLight(crntInstruction.coords, crntInstruction.propagateChannel, crntInstruction.value, chunk);
        }
        lightDataWasModified = true;
    }

    if (lightDataWasModified || chunk.uploadLightAnyway) {
        int index = chunk.neighbouringChunkIndices[13];
        memcpy((char*)lightDataBuffer.persistentMappedPtr + (index * sizeof(unsigned int) * chunk.lightData.data.size()), &chunk.lightData.data[0], chunk.lightData.data.size() * sizeof(unsigned int));
        chunk.uploadLightAnyway = false;
    }
}

int ChunkLighting::getBlockIndex(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE)
        return -1;
    if (y < 0 || y >= CHUNK_SIZE)
        return -1;
    if (z < 0 || z >= CHUNK_SIZE)
        return -1;
    return (x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + z;
}

void ChunkLighting::addNeighboursToQueue(const BlockCoords coords, int distance, ChunkDataContainer &chunk) {
    for (int i = 0; i < 6; i++) {
        int x = ((i / 2) % 3 == 0) * ((2 * (i % 2 == 0)) - 1);
        int y = ((i / 2) % 3 == 1) * ((2 * (i % 2 == 0)) - 1);
        int z = ((i / 2) % 3 == 2) * ((2 * (i % 2 == 0)) - 1);
        BlockCoords crntBlockCoords(coords.x + x, coords.y + y, coords.z + z);
        int index = getBlockIndex(crntBlockCoords.x, crntBlockCoords.y, crntBlockCoords.z);
        if (index != -1) {
            if (chunk.chunkLightDataBFSvisited[index] == false) {
                BFSqueue.push({chunk, crntBlockCoords, distance + 1});
                chunk.chunkLightDataBFSvisited[index] = true;
            }
        } else {
            ChunkDataContainer &neighbouringChunk = worldContainer.chunks[chunk.neighbouringChunkIndices[(9 * (x + 1)) + (3 * (y + 1)) + (z + 1)]];
            crntBlockCoords = BlockCoords(fastFloat::mod(crntBlockCoords.x, CHUNK_SIZE), fastFloat::mod(crntBlockCoords.y, CHUNK_SIZE), fastFloat::mod(crntBlockCoords.z, CHUNK_SIZE));
            index = getBlockIndex(crntBlockCoords.x, crntBlockCoords.y, crntBlockCoords.z);
            if (neighbouringChunk.chunkLightDataBFSvisited[index] == false) {
                BFSqueue.push({neighbouringChunk, crntBlockCoords, distance + 1});
                neighbouringChunk.chunkLightDataBFSvisited[index] = true;
            }
            if (neighbouringChunk.lightVisited == false) {
                visitedChunkIndices.push_back(neighbouringChunk.neighbouringChunkIndices[13]);
                neighbouringChunk.lightVisited = true;
            }
        }
    }
}

bool ChunkLighting::setLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunk) {
    int lightIndex = ((coords.x + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((coords.y + 1) * (CHUNK_SIZE + 2)) + (coords.z + 1);

    RGB previousLight(chunk.lightData.data[lightIndex]);
    unsigned int previousLightValue = chunk.lightData.data[lightIndex];

    unsigned int newLightValue = (std::max((unsigned char)(value * (channel == 0)), previousLight.r) << 16) + (std::max((unsigned char)(value * (channel == 1)), previousLight.g) << 8) + std::max((unsigned char)(value * (channel == 2)), previousLight.b);
    chunk.lightData.data[lightIndex] = chunk.lightData.data[lightIndex] & (!0xffffff);
    chunk.lightData.data[lightIndex] |= newLightValue;

    for (int i = 0; i < 27; i++) {
        if (i != 13) {
            int x = (i / 9) - 1;
            int y = ((i / 3) % 3) - 1;
            int z = (i % 3) - 1;

            x = coords.x - (x * CHUNK_SIZE);
            y = coords.y - (y * CHUNK_SIZE);
            z = coords.z - (z * CHUNK_SIZE);

            if ((x >= -1 && x <= CHUNK_SIZE) && (y >= -1 && y <= CHUNK_SIZE) && (z >= -1 && z <= CHUNK_SIZE)) {
                int lightIndex = ((x + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((y + 1) * (CHUNK_SIZE + 2)) + (z + 1);

                int neighbouringChunkIndex = chunk.neighbouringChunkIndices[i];
                worldContainer.chunks[neighbouringChunkIndex].lightData.data[lightIndex] = chunk.lightData.data[lightIndex] & 0xffffff;
                worldContainer.chunks[neighbouringChunkIndex].lightData.data[lightIndex] |= newLightValue;

                worldContainer.chunks[neighbouringChunkIndex].uploadLightAnyway = true;
            }
        }
    }

    return (previousLightValue != newLightValue);
}

void ChunkLighting::propagateLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunk) {
    BFSqueue.empty();

    bool doISearch = setLight(coords, channel, value, chunk);

    if (!doISearch)
        return;

    visitedChunkIndices.push_back(chunk.neighbouringChunkIndices[13]);
    chunk.lightVisited = true;
    chunk.chunkLightDataBFSvisited[getBlockIndex(coords.x, coords.y, coords.z)] = true;

    addNeighboursToQueue(coords, 0, chunk);

    while (BFSqueue.size() > 0) {
        ChunkDataContainer &crntChunk = BFSqueue.front().chunk;
        BlockCoords crntCoords = BFSqueue.front().coords;
        int distance = BFSqueue.front().distance;
        BFSqueue.pop();

        unsigned char newLight = (value - (15 * distance)) * ((value - (15 * distance)) > 0);

        int crntIndex = getBlockIndex(crntCoords.x, crntCoords.y, crntCoords.z);

        if (crntIndex == -1)
            std::cout << "You deserve to die, the lighting engine has bugs. (propagateLight)\n";

        if (blocks.blocks[crntChunk.blockAtCoords(crntCoords)].isSolid == false) {
            bool doISearch = setLight(crntCoords, channel, newLight, crntChunk);

            if (newLight != 0 && doISearch)
                addNeighboursToQueue(crntCoords, distance, crntChunk);
        }
    }
    for (auto i : visitedChunkIndices) {
        worldContainer.chunks[i].chunkLightDataBFSvisited = {0};
        worldContainer.chunks[i].lightVisited = false;
    }
    visitedChunkIndices.clear();
}


bool ChunkLighting::removeLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunk) {
    int lightIndex = ((coords.x + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((coords.y + 1) * (CHUNK_SIZE + 2)) + (coords.z + 1);

    RGB previousLight(chunk.lightData.data[lightIndex]);

    unsigned char previousVal = (previousLight.r * (channel == 0)) + (previousLight.g * (channel == 1)) + (previousLight.b * (channel == 2));

    if (previousVal > value) {
        LightUpdateInstruction instruction = {
            .propagationType = true,
            .propagateChannel = channel,
            .coords = coords,
            .value = previousVal
        };

        chunk.lightUpdateInstructions.insert(chunk.lightUpdateInstructions.begin(), instruction);
    }

    RGB newLight(previousLight.r * (channel != 0), previousLight.g * (channel != 1), previousLight.b * (channel != 2));

    unsigned int newLightValue = (newLight.r << 16) + (newLight.g << 8) + newLight.b;
    chunk.lightData.data[lightIndex] = chunk.lightData.data[lightIndex] & (!0xffffff);
    chunk.lightData.data[lightIndex] |= newLightValue;

    for (int i = 0; i < 27; i++) {
        if (i != 13) {
            int x = (i / 9) - 1;
            int y = ((i / 3) % 3) - 1;
            int z = (i % 3) - 1;

            x = coords.x - (x * CHUNK_SIZE);
            y = coords.y - (y * CHUNK_SIZE);
            z = coords.z - (z * CHUNK_SIZE);

            if ((x >= -1 && x <= CHUNK_SIZE) && (y >= -1 && y <= CHUNK_SIZE) && (z >= -1 && z <= CHUNK_SIZE)) {
                int lightIndex = ((x + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((y + 1) * (CHUNK_SIZE + 2)) + (z + 1);

                int neighbouringChunkIndex = chunk.neighbouringChunkIndices[i];
                worldContainer.chunks[neighbouringChunkIndex].lightData.data[lightIndex] = chunk.lightData.data[lightIndex] & 0xffffff;
                worldContainer.chunks[neighbouringChunkIndex].lightData.data[lightIndex] |= newLightValue;

                worldContainer.chunks[neighbouringChunkIndex].uploadLightAnyway = true;
            }
        }
    }

    return (value >= previousVal) && (previousVal != 0);
}

void ChunkLighting::depropagateLight(const BlockCoords coords, const char channel, const unsigned char value, ChunkDataContainer &chunk) {
    BFSqueue.empty();

    bool doISearch = removeLight(coords, channel, value, chunk);

    if (!doISearch)
        return;

    visitedChunkIndices.push_back(chunk.neighbouringChunkIndices[13]);
    chunk.lightVisited = true;
    chunk.chunkLightDataBFSvisited[getBlockIndex(coords.x, coords.y, coords.z)] = true;

    addNeighboursToQueue(coords, 0, chunk);;

    while (BFSqueue.size() > 0) {
        ChunkDataContainer &crntChunk = BFSqueue.front().chunk;
        BlockCoords crntCoords = BFSqueue.front().coords;
        int distance = BFSqueue.front().distance;
        BFSqueue.pop();

        unsigned char newLight = (value - (15 * distance)) * ((value - (15 * distance)) > 0);

        int crntIndex = getBlockIndex(crntCoords.x, crntCoords.y, crntCoords.z);

        if (crntIndex == -1)
            std::cout << "You deserve to die, the lighting engine has bugs. (depropagateLight)\n";

        if (blocks.blocks[crntChunk.blockAtCoords(crntCoords)].isSolid == false) {
            bool doISearch = removeLight(crntCoords, channel, newLight, crntChunk);

            if (newLight != 0 && doISearch)
                addNeighboursToQueue(crntCoords, distance, crntChunk);
        }
    }
    for (auto i : visitedChunkIndices) {
        worldContainer.chunks[i].chunkLightDataBFSvisited = {0};
        worldContainer.chunks[i].lightVisited = false;
    }
    visitedChunkIndices.clear();
}

void ChunkLighting::lightAO(ChunkDataContainer &chunk) {
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (blocks.blocks[chunk.blockAtCoords(i, j, k)].castsAO) {
                    int lightIndex = ((i + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((j + 1) * (CHUNK_SIZE + 2)) + (k + 1);
                    chunk.lightData.data[lightIndex] |= 1 << 30;

                    for (int i = 0; i < 27; i++) {
                        if (i != 13) {
                            int x = (i / 9) - 1;
                            int y = ((i / 3) % 3) - 1;
                            int z = (i % 3) - 1;

                            x = i - (x * CHUNK_SIZE);
                            y = j - (y * CHUNK_SIZE);
                            z = k - (z * CHUNK_SIZE);

                            if ((x >= -1 && x <= CHUNK_SIZE) && (y >= -1 && y <= CHUNK_SIZE) && (z >= -1 && z <= CHUNK_SIZE)) {
                                int lightIndex = ((x + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((y + 1) * (CHUNK_SIZE + 2)) + (z + 1);

                                int neighbouringChunkIndex = chunk.neighbouringChunkIndices[i];
                                worldContainer.chunks[neighbouringChunkIndex].lightData.data[lightIndex] |= 1 << 30;

                                worldContainer.chunks[neighbouringChunkIndex].uploadLightAnyway = true;
                            }
                        }
                    }
                }
            }
        }
    }
}