#include "../include/chunkList.h"

void ChunkList::updateLight(std::array<int, 3>& coords, int threadID) {
    int index = coordToIndexMap[coordsToString(coords)];
    for (int i = 0; i < CHUNK_SIZE + 1; i++) {
        for (int j = 0; j < CHUNK_SIZE + 1; j++) {
            for (int k = 0; k < CHUNK_SIZE + 1; k++) {
                //if (i % 2 == 1) {
                    chunkWorldContainer.at(index).lightData.at((i * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) + (j * (CHUNK_SIZE + 1)) + k) = 1.0f;
                //}
            }
        }
    }
}

void ChunkList::uploadLight(int index) {
    // std::cout << chunkWorldContainer.at(index).lightData.at(0) << "\n";
    lightDataOnGPU.Bind();
    lightDataOnGPU.Upload(chunkWorldContainer.at(index).lightData, sizeof(float) * index * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1), chunkWorldContainer.at(index).lightData.size() * sizeof(float), 3);
    lightDataOnGPU.Unbind();
}