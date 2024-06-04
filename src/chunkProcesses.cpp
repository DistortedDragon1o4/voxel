#include "../include/chunkList.h"
#include "buffers.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "fastFloat.h"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <array>
#include <chrono>
#include <ratio>
#include <string>
#include <thread>


VoxelGame::VoxelGame(int &width, int &height, const glm::dvec3 position, const std::string dir) :
	voxelShader(dir + "/shaders/vertex.glsl", dir + "/shaders/fragment.glsl"),
	voxelBlockTextureArray(1, "blocks/", 1, NUM_TEXTURES, dir + "/"),
	camera(width, height, position),
    blocks(dir),
	worldContainer(camera),
	rayCaster(worldContainer),
	highlightCursor(rayCaster, camera, dir),
	processManager(worldContainer, blocks, camera),
	interface(worldContainer, processManager, highlightCursor),
	renderer(voxelShader, voxelBlockTextureArray, camera, worldContainer, processManager.lighting, blocks, highlightCursor, dir) {
		voxelBlockTextureArray.TexUnit(voxelShader, "array", 1);
	}

// More stuff should be added here
VoxelGame::~VoxelGame() {
	voxelShader.Delete();
	voxelBlockTextureArray.Delete();
}

ChunkProcessManager::ChunkProcessManager(WorldContainer &worldContainer, BlockDefs &blocks, Camera &camera) :
	worldContainer(worldContainer),
	blocks(blocks),
	builder(worldContainer, blocks),
	lighting(worldContainer, blocks),
	camera(camera) {}


bool WorldContainer::isEdgeChunk(int coordX, int coordY, int coordZ) {
    if (coordX - fastFloat::fastFloor(camera.Position.x / CHUNK_SIZE) == RENDER_DISTANCE / 2 - 1 || fastFloat::fastFloor(camera.Position.x / CHUNK_SIZE) - coordX == RENDER_DISTANCE / 2 || coordY - fastFloat::fastFloor(camera.Position.y / CHUNK_SIZE) == RENDER_DISTANCE / 2 - 1 || fastFloat::fastFloor(camera.Position.y / CHUNK_SIZE) - coordY == RENDER_DISTANCE / 2 || coordZ - fastFloat::fastFloor(camera.Position.z / CHUNK_SIZE) == RENDER_DISTANCE / 2 - 1 || fastFloat::fastFloor(camera.Position.z / CHUNK_SIZE) - coordZ == RENDER_DISTANCE / 2) {
        return 1;
    }
    return 0;
}

void ChunkProcessManager::chunkPopulator() {
    while (run == 1) {
    	ChunkCoords cameraChunk;
        cameraChunk.x = int(floor(camera.Position.x / CHUNK_SIZE)) - (RENDER_DISTANCE / 2);
        cameraChunk.y = int(floor(camera.Position.y / CHUNK_SIZE)) - (RENDER_DISTANCE / 2);
        cameraChunk.z = int(floor(camera.Position.z / CHUNK_SIZE)) - (RENDER_DISTANCE / 2);
        if (firstRun == 0) {
        	int loadedChunkCoord[RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE][3];
            int offsetX = fastFloat::fastFloor(camera.Position.x / CHUNK_SIZE) - (RENDER_DISTANCE / 2);
            int offsetY = fastFloat::fastFloor(camera.Position.y / CHUNK_SIZE) - (RENDER_DISTANCE / 2);
            int offsetZ = fastFloat::fastFloor(camera.Position.z / CHUNK_SIZE) - (RENDER_DISTANCE / 2);
            for (int i = 0; i < RENDER_DISTANCE; i++) {
                for (int j = 0; j < RENDER_DISTANCE; j++) {
                    for (int k = 0; k < RENDER_DISTANCE; k++) {
                        loadedChunkCoord[(i * RENDER_DISTANCE * RENDER_DISTANCE) + (j * RENDER_DISTANCE) + k][0] = i + offsetX;
                        loadedChunkCoord[(i * RENDER_DISTANCE * RENDER_DISTANCE) + (j * RENDER_DISTANCE) + k][1] = j + offsetY;
                        loadedChunkCoord[(i * RENDER_DISTANCE * RENDER_DISTANCE) + (j * RENDER_DISTANCE) + k][2] = k + offsetZ;
                    }
                }
            }
            for (int i = 0; i < (RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE); i++) {
            	ChunkDataContainer &chunk = worldContainer.chunks[i];

                // Filling up the data of the chunk

                chunk.chunkID.x = loadedChunkCoord[i][0];
                chunk.chunkID.y = loadedChunkCoord[i][1];
                chunk.chunkID.z = loadedChunkCoord[i][2];

                worldContainer.coordToIndexMap[worldContainer.coordsToKey(chunk.chunkID)] = i;

                chunk.emptyChunk = 0;
                chunk.unGeneratedChunk = 1;
                chunk.unCompiledChunk = 1;
                chunk.renderlck = 1;

                // Filling up neighbouring chunk indices

                for (int j = 0; j < 27; j++) {
                    int z = j % 3;
                    int y = (j / 3) % 3;
                    int x = j / 9;

                    int crntNeighbour = worldContainer.getIndex(chunk.chunkID.x + x - 1, chunk.chunkID.y + y - 1, chunk.chunkID.z + z - 1);
                    if (crntNeighbour != -1) {
                        chunk.neighbouringChunkIndices[j] = crntNeighbour;

                        worldContainer.chunks[crntNeighbour].neighbouringChunkIndices[26 - j] = i;
                    } else {
                        chunk.neighbouringChunkIndices[j] = -1;
                    }
                }
            }
            firstRun = 1;
        } else {
            for (int i = 0; i < (RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE); i++) {
            	ChunkDataContainer &chunk = worldContainer.chunks[i];

                // Bad name, it's basically the coords of the chunk w.r.t the (bottom, rear, left)most chunk
                std::array<int, 3> axisDistances {
                    chunk.chunkID.x - cameraChunk.x,
                    chunk.chunkID.y - cameraChunk.y,
                    chunk.chunkID.z - cameraChunk.z,
                };


                // Checking if the current chunk is outside the renderred area and needs to be re-allocated
                ChunkCoords newChunkID;
                bool x = false;
                if (!(axisDistances[0] < RENDER_DISTANCE && axisDistances[0] >= 0)) {
                    int m = fastFloat::mod(axisDistances[0], RENDER_DISTANCE);
                    newChunkID.x = cameraChunk.x + m;
                    x = true;
                } else {
                	newChunkID.x = chunk.chunkID.x;
                }

                if (!(axisDistances[1] < RENDER_DISTANCE && axisDistances[1] >= 0)) {
                    int m = fastFloat::mod(axisDistances[1], RENDER_DISTANCE);
                    newChunkID.y = cameraChunk.y + m;
                    x = true;
                } else {
                	newChunkID.y = chunk.chunkID.y;
                }

                if (!(axisDistances[2] < RENDER_DISTANCE && axisDistances[2] >= 0)) {
                    int m = fastFloat::mod(axisDistances[2], RENDER_DISTANCE);
                    newChunkID.z = cameraChunk.z + m;
                    x = true;
                } else {
                	newChunkID.z = chunk.chunkID.z;
                }

                // Re-allocating the chunk
                if (x) {

                    // Yeeting it from the hashmap
                    worldContainer.coordToIndexMap.erase(worldContainer.coordsToKey(chunk.chunkID));

                    worldContainer.coordToIndexMap[worldContainer.coordsToKey(newChunkID)] = i;

                    // Yeeting and re-filling its neighbouring chunk indices
                    for (int j = 0; j < 27; j++) {
                        int z = j % 3;
                        int y = (j / 3) % 3;
                        int x = j / 9;

                        int prevNeighbour = worldContainer.getIndex(chunk.chunkID.x + x - 1, chunk.chunkID.y + y - 1, chunk.chunkID.z + z - 1);
                        if (prevNeighbour != -1) {
                            worldContainer.chunks[prevNeighbour].neighbouringChunkIndices[26 - j] = -1;
                        }

                        int newNeighbour = worldContainer.getIndex(newChunkID.x + x - 1, newChunkID.y + y - 1, newChunkID.z + z - 1);
                        if (newNeighbour != -1) {
                            chunk.neighbouringChunkIndices[j] = newNeighbour;

                            worldContainer.chunks[newNeighbour].neighbouringChunkIndices[26 - j] = i;
                        } else {
                            chunk.neighbouringChunkIndices[j] = -1;
                        }
                    }

                    // Giving it new data
                    chunk.chunkID = newChunkID;

                    chunk.unGeneratedChunk = 1;
                    chunk.unCompiledChunk = 1;
                    chunk.meshSize = 0;
                    chunk.chunkData.clear();
                    chunk.lightData.clear();
                    chunk.lightUpdateInstructions.empty();

                    chunk.emptyChunk = 0;
                    chunk.reUploadMesh = false;

                    chunk.isSingleBlock = false;
                    chunk.theBlock = 0;
                }
            }
        }

        cameraChunk.x = int(floor(camera.Position.x / CHUNK_SIZE));
        cameraChunk.y = int(floor(camera.Position.y / CHUNK_SIZE));
        cameraChunk.z = int(floor(camera.Position.z / CHUNK_SIZE));


        // const auto start = std::chrono::high_resolution_clock::now();
        if (cameraChunk.x != prevCameraChunk.x || cameraChunk.y != prevCameraChunk.y || cameraChunk.z != prevCameraChunk.z) {
            for (ChunkDataContainer &chunk : worldContainer.chunks) {
                // float distance = sqrt(pow((worldContainer.chunks[i].chunkID[0] * CHUNK_SIZE) - camPosX + (CHUNK_SIZE / 2), 2) + pow((worldContainer.chunks[i].chunkID[1] * CHUNK_SIZE) - camPosY + (CHUNK_SIZE / 2), 2) + pow((worldContainer.chunks[i].chunkID[2] * CHUNK_SIZE) - camPosZ + (CHUNK_SIZE / 2), 2));
                float distance = std::max(std::max(abs((chunk.chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)), abs(((chunk.chunkID.y * CHUNK_SIZE) - camera.Position.y + (CHUNK_SIZE / 2)))), abs((chunk.chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2)));
                chunk.distance = distance;
            }
        }

        prevCameraChunk = cameraChunk;
        // const auto end = std::chrono::high_resolution_clock::now();
 
        // const std::chrono::duration<double> diff = end - start;
        // std::cout << "BFS search: " << diff << "\n";

        // std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void ChunkProcessManager::buildChunks() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    while (run == 1) {
    	int index = 0;

        bool iLetYouRun = false;
        
        if (chunkUpdateQueue.size() > 0) {
        	index = chunkUpdateQueue.front();
            if (worldContainer.chunks.at(index).unGeneratedChunk == false)
                iLetYouRun = true;
            chunkUpdateQueue.pop();
        } else {
            for (int i = 0; i < worldContainer.chunks.size(); i++) {
                if (worldContainer.chunks[i].distance <= worldContainer.chunks[index].distance && worldContainer.chunks[i].unGeneratedChunk == 0 && worldContainer.chunks[i].unCompiledChunk == 1) {
                    index = i;
                    iLetYouRun = true;
                }
            }
        }

        ChunkDataContainer &chunk = worldContainer.chunks[index];
        
        if (iLetYouRun) {
            if ((chunk.unCompiledChunk == 1 || chunk.forUpdate == 1) && chunk.unGeneratedChunk == 0) {
                lighting.lightAO(chunk);

                int state = builder.buildChunk(chunk);
            }
        }
    }
}

void ChunkProcessManager::generateChunks() {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    while (run == 1) {
        int index = 0;
        for (int i = 0; i < worldContainer.chunks.size(); i++) {
            if (worldContainer.chunks[i].distance <= worldContainer.chunks[index].distance && worldContainer.chunks[i].unGeneratedChunk == 1) {
                index = i;
            }
        }

        if (worldContainer.chunks.at(index).unGeneratedChunk == true) {

            generator.generateChunk(worldContainer.chunks[index].chunkData, worldContainer.chunks[index].chunkID);
            
            worldContainer.chunks[index].unGeneratedChunk = 0;
            worldContainer.chunks[index].unCompiledChunk = 1;

        }
    }
}

void ChunkProcessManager::updateChunk(ChunkCoords chunkCoords, bool surroundings) {
	int index = worldContainer.getIndex(chunkCoords);
	if (worldContainer.chunks.at(index).forUpdate == false) {
		// checkPermeability(worldContainer.chunks.at(index));
		worldContainer.chunks.at(index).forUpdate = true;
		chunkUpdateQueue.push(index);
	}
    if (surroundings) {
    	std::array<unsigned int, 6> neighbouringChunkIndices;

	    neighbouringChunkIndices.at(0) = worldContainer.getIndex(chunkCoords.x + 1, chunkCoords.y, chunkCoords.z);

	    neighbouringChunkIndices.at(1) = worldContainer.getIndex(chunkCoords.x - 1, chunkCoords.y, chunkCoords.z);

	    neighbouringChunkIndices.at(4) = worldContainer.getIndex(chunkCoords.x, chunkCoords.y, chunkCoords.z + 1);

	    neighbouringChunkIndices.at(5) = worldContainer.getIndex(chunkCoords.x, chunkCoords.y, chunkCoords.z - 1);

	    neighbouringChunkIndices.at(2) = worldContainer.getIndex(chunkCoords.x, chunkCoords.y + 1, chunkCoords.z);

	    neighbouringChunkIndices.at(3) = worldContainer.getIndex(chunkCoords.x, chunkCoords.y - 1, chunkCoords.z);

	    for (int chunkIndex : neighbouringChunkIndices) {
	    	if (chunkIndex != -1) {
	    		if (worldContainer.chunks.at(chunkIndex).forUpdate == false) {
		    		chunkUpdateQueue.push(chunkIndex);
		    		worldContainer.chunks.at(chunkIndex).forUpdate = true;
	    		}
	    	}
	    }
    }
}