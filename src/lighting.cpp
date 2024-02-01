#include "../include/chunkList.h"
#include "chunkDataContainer.h"

ChunkLighting::ChunkLighting(WorldContainer &worldContainer):
    worldContainer(worldContainer) {
        lightDataBuffer.create();
        lightDataBuffer.allocate(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE * sizeof(ChunkLightContainer), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        lightDataBuffer.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        lightDataBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
    }

void ChunkLighting::updateLight(const ChunkCoords coords) {
    int index = worldContainer.getIndex(coords);
    if (index == -1)
        return;
    ChunkDataContainer &chunk = worldContainer.chunks[index];

    for (int i = 0; i < CHUNK_SIZE + 2; i++) {
        for (int j = 0; j < CHUNK_SIZE + 2; j++) {
            for (int k = 0; k < CHUNK_SIZE + 2; k++) {
                // if (i == j) {
                    chunk.lightData.data[(i * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + (j * (CHUNK_SIZE + 2)) + k] = 0x808080;
                // }
            }
        }
    }

    memcpy((char*)lightDataBuffer.persistentMappedPtr + (index * sizeof(int) * chunk.lightData.data.size()), &chunk.lightData.data, sizeof(ChunkLightContainer));

    // chunk.lightUploaded = false;
    // chunk.lightReadyToUpload = true;
}

// void ChunkLighting::uploadLight(int index) {
//     // std::cout << chunkWorldContainer.at(index).lightData.at(0) << "\n";

//     ChunkDataContainer &chunk = worldContainer.chunks[index];

//     // memcpy((char*)lightDataBuffer.persistentMappedPtr + (index * sizeof(int) * chunk.lightData.data.size()), &chunk.lightData.data, sizeof(ChunkLightContainer));
//     chunk.lightUploaded = true;
//     chunk.lightReadyToUpload = false;
// }