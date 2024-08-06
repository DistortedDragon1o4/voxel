#include "../include/chunkList.h"
#include "buffers.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "coordinateContainers.h"
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
	voxelBlockTextureArray(0, "blocks/", 1, NUM_TEXTURES, dir + "/"),
	camera(width, height, position),
    blocks(dir),
	worldContainer(simulationRadius, simulationHeight, renderRadius, renderHeight),
	rayCaster(worldContainer),
	highlightCursor(rayCaster, camera, dir),
	processManager(worldContainer, blocks, camera, dir),
	interface(worldContainer, processManager, highlightCursor),
	renderer(voxelShader, voxelBlockTextureArray, camera, worldContainer, processManager.lighting, blocks, dir) {
		voxelBlockTextureArray.TexUnit(voxelShader, "array", 0);
	}

// More stuff should be added here
VoxelGame::~VoxelGame() {
	voxelShader.Delete();
	voxelBlockTextureArray.Delete();
}

ChunkProcessManager::ChunkProcessManager(WorldContainer &worldContainer, BlockDefs &blocks, Camera &camera, const std::string dir) :
	worldContainer(worldContainer),
	blocks(blocks),
	builder(worldContainer, blocks),
	lighting(worldContainer, blocks),
    // saver(dir),
	camera(camera) {}

void ChunkProcessManager::chunkPopulator() {
    while (run == 1) {
        ChunkCoords chunkContainingCamera;
        chunkContainingCamera.x = int(floor(camera.Position.x / CHUNK_SIZE));
        chunkContainingCamera.y = int(floor(camera.Position.y / CHUNK_SIZE));
        chunkContainingCamera.z = int(floor(camera.Position.z / CHUNK_SIZE));

        ChunkCoords firstTickableChunk;
        firstTickableChunk.x = chunkContainingCamera.x - worldContainer.simulationRadius - 1;
        firstTickableChunk.y = chunkContainingCamera.y - worldContainer.simulationHeight - 1;
        firstTickableChunk.z = chunkContainingCamera.z - worldContainer.simulationRadius - 1;

        ChunkCoords firstRenderableChunk;
        firstRenderableChunk.x = chunkContainingCamera.x - worldContainer.renderRadius - 1;
        firstRenderableChunk.y = chunkContainingCamera.y - worldContainer.renderHeight - 1;
        firstRenderableChunk.z = chunkContainingCamera.z - worldContainer.renderRadius - 1;

        ChunkCoords lastTickableChunk;
        lastTickableChunk.x = chunkContainingCamera.x + worldContainer.simulationRadius;
        lastTickableChunk.y = chunkContainingCamera.y + worldContainer.simulationHeight;
        lastTickableChunk.z = chunkContainingCamera.z + worldContainer.simulationRadius;

        ChunkCoords lastRenderableChunk;
        lastRenderableChunk.x = chunkContainingCamera.x + worldContainer.renderRadius;
        lastRenderableChunk.y = chunkContainingCamera.y + worldContainer.renderHeight;
        lastRenderableChunk.z = chunkContainingCamera.z + worldContainer.renderRadius;

        if (chunkContainingCamera.x != prevCameraChunk.x || chunkContainingCamera.y != prevCameraChunk.y || chunkContainingCamera.z != prevCameraChunk.z || firstRun == 0) {
            if (firstRun == 0) {
                for (int i = 0; i < worldContainer.chunks.size(); i++) {
                    ChunkDataContainer &chunk = worldContainer.chunks[i];

                    chunk.chunkID.x = ((i % (4 * (worldContainer.renderRadius + 1) * (worldContainer.renderRadius + 1))) / (2 * (worldContainer.renderRadius + 1))) + firstRenderableChunk.x;
                    chunk.chunkID.y = (i / (4 * (worldContainer.renderRadius + 1) * (worldContainer.renderRadius + 1))) + firstRenderableChunk.y;
                    chunk.chunkID.z = (i % (2 * (worldContainer.renderRadius + 1))) + firstRenderableChunk.z;

                    worldContainer.coordToIndexMap[worldContainer.coordsToKey(chunk.chunkID)] = i;

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

                    chunk.emptyChunk = false;

                    chunk.unGeneratedChunk = true;
                    chunk.unCompiledChunk = true;

                    float distance = std::max(std::max(abs((chunk.chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)), abs(((chunk.chunkID.y * CHUNK_SIZE) - camera.Position.y + (CHUNK_SIZE / 2)))), abs((chunk.chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2)));
                    chunk.distance = distance;
                    chunk.euclideanDistSquared =      (((chunk.chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)) * ((chunk.chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)))
                                                    + (((chunk.chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2)) * ((chunk.chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2)));

                    if ((firstTickableChunk.x <= chunk.chunkID.x && chunk.chunkID.x <= lastTickableChunk.x) && (firstTickableChunk.y <= chunk.chunkID.y && chunk.chunkID.y <= lastTickableChunk.y) && (firstTickableChunk.z <= chunk.chunkID.z && chunk.chunkID.z <= lastTickableChunk.z)) {
                        chunk.chunkType = 0;
                        // chunk.lightData.isEmpty = true;
                    }

                    unsigned int prevChunkLODlevel = chunk.chunkData.lodLevel;
                    if (chunk.euclideanDistSquared <= worldContainer.LOD0distSquared)
                        chunk.chunkData.lodLevel = 0;
                    else if (chunk.euclideanDistSquared <= worldContainer.LOD1distSquared)
                        chunk.chunkData.lodLevel = 1;
                    else if (chunk.euclideanDistSquared <= worldContainer.LOD2distSquared)
                        chunk.chunkData.lodLevel = 2;
                    else
                        chunk.chunkData.lodLevel = 3;
                }
                firstRun = true;
            } else {
                for (int i = 0; i < worldContainer.chunks.size(); i++) {
                    ChunkDataContainer &chunk = worldContainer.chunks[i];

                    // Bad name, it's basically the coords of the chunk w.r.t the (bottom, rear, left)most chunk
                    ChunkCoords axisDistances {
                        .x = chunk.chunkID.x - firstRenderableChunk.x,
                        .y = chunk.chunkID.y - firstRenderableChunk.y,
                        .z = chunk.chunkID.z - firstRenderableChunk.z,
                    };


                    // Checking if the current chunk is outside the renderred area and needs to be re-allocated
                    ChunkCoords newChunkID;
                    bool x = false;
                    if (!(axisDistances.x < 2 * (worldContainer.renderRadius + 1) && axisDistances.x >= 0)) {
                        int m = fastFloat::mod(axisDistances.x, 2 * (worldContainer.renderRadius + 1));
                        newChunkID.x = firstRenderableChunk.x + m;
                        x = true;
                    } else {
                        newChunkID.x = chunk.chunkID.x;
                    }

                    if (!(axisDistances.y < 2 * (worldContainer.renderHeight + 1) && axisDistances.y >= 0)) {
                        int m = fastFloat::mod(axisDistances.y, 2 * (worldContainer.renderHeight + 1));
                        newChunkID.y = firstRenderableChunk.y + m;
                        x = true;
                    } else {
                        newChunkID.y = chunk.chunkID.y;
                    }

                    if (!(axisDistances.z < 2 * (worldContainer.renderRadius + 1) && axisDistances.z >= 0)) {
                        int m = fastFloat::mod(axisDistances.z, 2 * (worldContainer.renderRadius + 1));
                        newChunkID.z = firstRenderableChunk.z + m;
                        x = true;
                    } else {
                        newChunkID.z = chunk.chunkID.z;
                    }

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

                        chunk.lodRecompilation = false;
                        chunk.unGeneratedChunk = true;
                        chunk.unCompiledChunk = true;
                        chunk.meshSize = 0;
                        chunk.chunkData.clear();
                        chunk.lightData.clear();
                        chunk.lightData.isEmpty = true;
                        chunk.lightUpdateInstructions.empty();

                        chunk.contentsAreUseless = false;
                        chunk.meshIsUseless = false;
                        chunk.lightingIsUseless = false;

                        for (auto &i : chunk.neighbouringChunkIndices) {
                            worldContainer.chunks[i].unGeneratedChunk = true;
                        }

                        chunk.emptyChunk = false;
                        chunk.reUploadMesh = false;
                    }

                    float distance = std::max(std::max(abs((chunk.chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)), abs(((chunk.chunkID.y * CHUNK_SIZE) - camera.Position.y + (CHUNK_SIZE / 2)))), abs((chunk.chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2)));
                    chunk.distance = distance;
                    chunk.euclideanDistSquared =      (((chunk.chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)) * ((chunk.chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)))
                                                    + (((chunk.chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2)) * ((chunk.chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2)));

                    if ((firstTickableChunk.x <= chunk.chunkID.x && chunk.chunkID.x <= lastTickableChunk.x) && (firstTickableChunk.y <= chunk.chunkID.y && chunk.chunkID.y <= lastTickableChunk.y) && (firstTickableChunk.z <= chunk.chunkID.z && chunk.chunkID.z <= lastTickableChunk.z)) {
                        if (chunk.chunkType > 0) {
                            chunk.unGeneratedChunk = true;
                            // chunk.unCompiledChunk = true;
                        }
                        chunk.chunkType = 0;
                        // chunk.chunkData.lodLevel = 0;
                        chunk.contentsAreUseless = false;
                        chunk.meshIsUseless = false;
                        chunk.lightingIsUseless = false;
                    } else {
                        // if (chunk.chunkType != 1)
                        //     chunk.unCompiledChunk = true;
                        chunk.chunkType = 1;
                        // chunk.chunkData.lodLevel = 1;
                    }

                    unsigned int prevChunkLODlevel = chunk.chunkData.lodLevel;
                    if (chunk.euclideanDistSquared <= worldContainer.LOD0distSquared)
                        chunk.chunkData.lodLevel = 0;
                    else if (chunk.euclideanDistSquared <= worldContainer.LOD1distSquared)
                        chunk.chunkData.lodLevel = 1;
                    else if (chunk.euclideanDistSquared <= worldContainer.LOD2distSquared)
                        chunk.chunkData.lodLevel = 2;
                    else
                        chunk.chunkData.lodLevel = 3;

                    if (chunk.chunkData.lodLevel != prevChunkLODlevel) {
                        chunk.lodRecompilation = true;
                        chunk.unCompiledChunk = true;
                        if (chunk.chunkType == 1) {
                            chunk.unGeneratedChunk = true;
                            chunk.contentsAreUseless = false;
                            chunk.meshIsUseless = false;
                            chunk.lightingIsUseless = false;
                            for (auto &i : chunk.neighbouringChunkIndices) {
                                worldContainer.chunks[i].unGeneratedChunk = true;
                            }
                        }
                    }
                }
            }
        }
        for (auto &chunk : worldContainer.chunks) {
            if (chunk.chunkType == 1 && chunk.contentsAreUseless) {
                for (int j = 0; j < 27; j++) {
                    int neighbouringIndex = chunk.neighbouringChunkIndices[j];
                    if (neighbouringIndex != -1) {
                        if (worldContainer.chunks[neighbouringIndex].chunkType != 1 || !worldContainer.chunks[neighbouringIndex].contentsAreUseless)
                            goto end;
                    }
                }

                chunk.chunkData.clear();
                chunk.lightData.clear();
                chunk.lightData.isEmpty = true;

                end:
            }
        }    


        // const auto start = std::chrono::high_resolution_clock::now();

        prevCameraChunk = chunkContainingCamera;

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
                ChunkDataContainer &chunk = worldContainer.chunks[i];
                if (chunk.distance <= worldContainer.chunks[index].distance && chunk.unGeneratedChunk == false && chunk.unCompiledChunk == true) {
                    for (int i : chunk.neighbouringChunkIndices) {
                        if (i == -1) {
                            chunk.unCompiledChunk = 1;
                            chunk.meshSize = 0;
                            goto skipUpdatingValue;
                        }
                    }
                    index = i;
                    iLetYouRun = true;

                    skipUpdatingValue:
                }
            }
        }

        ChunkDataContainer &chunk = worldContainer.chunks[index];
        
        if (iLetYouRun) {
            if ((chunk.unCompiledChunk == 1 || chunk.forUpdate == 1) && chunk.unGeneratedChunk == 0) {
                // lighting.lightAO(chunk);

                // const auto start = std::chrono::high_resolution_clock::now();

                int state = builder.buildChunk(chunk);
                chunk.reUploadLight = true;

                // const auto end = std::chrono::high_resolution_clock::now();
 
                // const std::chrono::duration<double> diff = end - start;
                // std::cout << "Meshing: " << diff << "\n";
            }
        }
    }
}

void ChunkProcessManager::generateChunks() {
    // std::this_thread::sleep_for(std::chrono::milliseconds(250));
    while (run == 1) {
        int index = 0;
        for (int i = 0; i < worldContainer.chunks.size(); i++) {
            // std::cout << "Info of " << worldContainer.chunks[i].chunkID.x << " " << worldContainer.chunks[i].chunkID.y << " " << worldContainer.chunks[i].chunkID.z << " unGenChk " << worldContainer.chunks[i].unGeneratedChunk << " " << worldContainer.chunks[i].distance << "\n";
            if (worldContainer.chunks[i].distance <= worldContainer.chunks[index].distance && worldContainer.chunks[i].unGeneratedChunk == true) {
                index = i;
            }
        }

        if (worldContainer.chunks.at(index).unGeneratedChunk == true) {

            // std::cout << "Generated " << worldContainer.chunks[index].chunkID.x << " " << worldContainer.chunks[index].chunkID.y << " " << worldContainer.chunks[index].chunkID.z << "\n";

            generator.generateChunk(worldContainer.chunks[index].chunkData, worldContainer.chunks[index].chunkID);
            
            worldContainer.chunks[index].unGeneratedChunk = false;

            if (worldContainer.chunks[index].lightData.isEmpty && worldContainer.chunks[index].chunkType == 0) {
                worldContainer.chunks[index].lightData.init();
                worldContainer.chunks[index].lightData.isEmpty = false;
            }
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