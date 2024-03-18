#include "../include/chunkGenerator.h"

ChunkGen::ChunkGen() {
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
        chunk.push_back(0);
        bfs.push_back(false);
        light.push_back(16);
    }
}

void ChunkGen::generateChunk(std::vector<short> &chunk, int coordX, int coordY, int coordZ) {
    std::vector <float> dataContainer(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    // std::vector <float> dataContainer2(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    // std::vector <float> dataContainer3(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE); 

    fnGenerator->GenUniformGrid3D(dataContainer.data(), coordX * CHUNK_SIZE, coordY * CHUNK_SIZE, coordZ * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, frequency, seed);
    // fnGenerator2->GenUniformGrid3D(dataContainer2.data(), coordX * CHUNK_SIZE, coordY * CHUNK_SIZE, coordZ * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, frequency * 4, seed);
    // fnGenerator3->GenUniformGrid3D(dataContainer3.data(), coordX * CHUNK_SIZE, coordY * CHUNK_SIZE, coordZ * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, frequency * 2, seed + 500);

    int index = 0;

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                // if (i + j + k < CHUNK_SIZE)
                //     chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 2;
                /*if (fastFloat::fastCeil(dataContainer2[index]) == 1) {
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 4;
                    if (fastFloat::fastCeil(dataContainer[index]) == 1)
                        chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 2;
                if (fastFloat::fastCeil(dataContainer3[index]) == 1)
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 3;
                }*/

                // if (dataContainer[index] <= 0) {
                //     chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 1;
                //     // if (dataContainer2[index] < 0)
                //     //     chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 4;
                //     // if (dataContainer3[index] < 0)
                //     //     chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 2;
                // }

                if (coordY < 0)
                    chunk[(k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i] = 2;

                // if (i == 5)
                //     chunk.at((k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i) = 3;

                // if (dataContainer3[index] < 0)
                //     chunk.at((k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i) = 3;
                // if ((i == 0 && j == 0) || (j == 0 && k == 0) || (k == 0 && i == 0) || (i == CHUNK_SIZE - 1 && j == CHUNK_SIZE - 1) || (j == CHUNK_SIZE - 1 && k == CHUNK_SIZE - 1) || (k == CHUNK_SIZE - 1 && i == CHUNK_SIZE - 1))
                //     chunk.at((k * CHUNK_SIZE * CHUNK_SIZE) + (j * CHUNK_SIZE) + i) = 2;

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
