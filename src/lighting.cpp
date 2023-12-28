#include "../include/chunkList.h"
#include "chunkDataContainer.h"

void ChunkLighting::updateLight(const ChunkCoords coords) {
    int index = worldContainer.getIndex(coords);
    ChunkDataContainer &chunk = worldContainer.chunks[index];
    for (int i = 0; i < CHUNK_SIZE + 1; i++) {
        for (int j = 0; j < CHUNK_SIZE + 1; j++) {
            for (int k = 0; k < CHUNK_SIZE + 1; k++) {
                //if (i % 2 == 1) {
                    chunk.lightData[(i * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) + (j * (CHUNK_SIZE + 1)) + k] = 1.0f;
                //}
            }
        }
    }
}

void ChunkLighting::uploadLight(int index) {
    // std::cout << chunkWorldContainer.at(index).lightData.at(0) << "\n";
    lightDataOnGPU.Bind();
    lightDataOnGPU.Upload(worldContainer.chunks.at(index).lightData, sizeof(float) * index * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1), worldContainer.chunks.at(index).lightData.size() * sizeof(float), 3);
    lightDataOnGPU.Unbind();
}