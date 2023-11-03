#include "../include/chunkGenerator.h"

ChunkGen::ChunkGen() {
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
        chunk.push_back(0);
        light.push_back(16);
    }
}

void ChunkGen::generateChunk(std::vector<short> &chunk, int coordX, int coordY, int coordZ) {
    std::vector <float> dataContainer(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    std::vector <float> dataContainer2(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    std::vector <float> dataContainer3(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE); 

    fnGenerator->GenUniformGrid3D(dataContainer.data(), coordX * CHUNK_SIZE, coordY * CHUNK_SIZE, coordZ * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, frequency, seed);
    fnGenerator2->GenUniformGrid3D(dataContainer2.data(), coordX * CHUNK_SIZE, coordY * CHUNK_SIZE, coordZ * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, frequency, seed);
    fnGenerator3->GenUniformGrid3D(dataContainer3.data(), coordX * CHUNK_SIZE, coordY * CHUNK_SIZE, coordZ * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, frequency * 2, seed + 500);

    int index = 0;

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                /*if (coordY < -1)
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 1;*/
                /*if (fastFloat::fastCeil(dataContainer2[index]) == 1) {
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 4;
                    if (fastFloat::fastCeil(dataContainer[index]) == 1)
                        chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 2;
                if (fastFloat::fastCeil(dataContainer3[index]) == 1)
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 3;
                }*/
                if (dataContainer[index] <= 0) {
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 2;
                    if (dataContainer2[index] < 0)
                        chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 4;
                    if (dataContainer3[index] < 0)
                        chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 3;
                }
                /*if (coordY > -1)
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 0;*/
                index++;
            }
        }
    }
}

void ChunkGen::initChunk(std::vector<short> &chunk) {
    chunk = ChunkGen::chunk;
}
