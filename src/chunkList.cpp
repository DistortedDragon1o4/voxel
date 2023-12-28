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


VoxelGame::VoxelGame(const int width, const int height, const glm::dvec3 position, const std::string dir) :
	voxelShader(dir + "/shaders/vertex.glsl", dir + "/shaders/fragment.glsl"),
	voxelBlockTextureArray(0, "blocks/", 1, NUM_BLOCKS, dir + "/"),
	camera(width, height, position),
	worldContainer(camera),
	rayCaster(worldContainer),
	highlightCursor(rayCaster, camera, dir),
	permeability(worldContainer, blocks),
	queueGenerator(worldContainer, permeability),
	genQueueGenerator(worldContainer),
	frustumCuller(worldContainer, camera),
	processManager(worldContainer, blocks, permeability, queueGenerator, genQueueGenerator, frustumCuller, regionContainer, camera),
	interface(worldContainer, processManager, highlightCursor),
	renderer(voxelShader, voxelBlockTextureArray, voxelBlockTextureSampler, camera, worldContainer, regionContainer, processManager.lighting, blocks, frustumCuller, dir) {
		voxelBlockTextureArray.TexUnit(voxelShader, "array", 0);
	}

// More stuff should be added here
VoxelGame::~VoxelGame() {
	voxelShader.Delete();
	voxelBlockTextureArray.Delete();
}

ChunkProcessManager::ChunkProcessManager(WorldContainer &worldContainer, BlockDefs &blocks, ChunkPermeability &permeability, ChunkMeshingQueueGenerator &queueGenerator, ChunkGeneratingQueueGenerator &genQueueGenerator, FrustumCuller &frustumCuller, RegionContainer &regionContainer, Camera &camera) :
	worldContainer(worldContainer),
	blocks(blocks),
	builder(worldContainer, blocks),
	lighting(worldContainer),
	permeability(permeability),
	queueGenerator(queueGenerator),
	genQueueGenerator(genQueueGenerator),
	frustumCuller(frustumCuller),
	regionContainer(regionContainer),
	camera(camera) {}


bool WorldContainer::isEdgeChunk(int coordX, int coordY, int coordZ) {
    if (coordX - fastFloat::fastFloor(camera.Position.x / CHUNK_SIZE) == RENDER_DISTANCE / 2 - 1 || fastFloat::fastFloor(camera.Position.x / CHUNK_SIZE) - coordX == RENDER_DISTANCE / 2 || coordY - fastFloat::fastFloor(camera.Position.y / CHUNK_SIZE) == RENDER_DISTANCE / 2 - 1 || fastFloat::fastFloor(camera.Position.y / CHUNK_SIZE) - coordY == RENDER_DISTANCE / 2 || coordZ - fastFloat::fastFloor(camera.Position.z / CHUNK_SIZE) == RENDER_DISTANCE / 2 - 1 || fastFloat::fastFloor(camera.Position.z / CHUNK_SIZE) - coordZ == RENDER_DISTANCE / 2) {
        return 1;
    }
    return 0;
}

void ChunkProcessManager::calculateLoadedChunks() {
    frustumCuller.frustumOffset = (double(CHUNK_SIZE) / 2) / tan(frustumCuller.FOV / 2);
    frustumCuller.cosineModifiedHalfFOV = cos(atan(frustumCuller.screenDiag / (frustumCuller.screenHeight / tan(frustumCuller.FOV / 2))));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (run == 1) {
    	frustumCuller.cosineModifiedHalfFOV = cos(atan(frustumCuller.screenDiag / (frustumCuller.screenHeight / tan(frustumCuller.FOV / 2))));
        for (int i = 0; i < (RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE); i++) {
            // float distance = sqrt(pow((worldContainer.chunks[i].chunkID[0] * CHUNK_SIZE) - camPosX + (CHUNK_SIZE / 2), 2) + pow((worldContainer.chunks[i].chunkID[1] * CHUNK_SIZE) - camPosY + (CHUNK_SIZE / 2), 2) + pow((worldContainer.chunks[i].chunkID[2] * CHUNK_SIZE) - camPosZ + (CHUNK_SIZE / 2), 2));
            float distance = abs((worldContainer.chunks[i].chunkID.x * CHUNK_SIZE) - camera.Position.x + (CHUNK_SIZE / 2)) + abs(((worldContainer.chunks[i].chunkID.y * CHUNK_SIZE) - camera.Position.y + (CHUNK_SIZE / 2))) + abs((worldContainer.chunks[i].chunkID.z * CHUNK_SIZE) - camera.Position.z + (CHUNK_SIZE / 2));
            worldContainer.chunks[i].distance = distance;
            worldContainer.chunks.at(i).frustumVisible = frustumCuller.isFrustumCulled(worldContainer.chunks.at(i).chunkID);
        }
    }
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

                chunk.chunkID.x = loadedChunkCoord[i][0];
                chunk.chunkID.y = loadedChunkCoord[i][1];
                chunk.chunkID.z = loadedChunkCoord[i][2];
                worldContainer.coordToIndexMap[worldContainer.coordsToKey(chunk.chunkID)] = i;
                chunk.myIndex = i;
                chunk.emptyChunk = 0;
                chunk.unGeneratedChunk = 1;
                chunk.unCompiledChunk = 1;
                chunk.renderlck = 1;

                ChunkCoords crntRegionID;
                crntRegionID.x = floor(float(chunk.chunkID.x) / float(REGION_SIZE));
				crntRegionID.y = floor(float(chunk.chunkID.y) / float(REGION_SIZE));
				crntRegionID.z = floor(float(chunk.chunkID.z) / float(REGION_SIZE));

				int chunkIndex = (fastFloat::mod(chunk.chunkID.x, REGION_SIZE) * REGION_SIZE * REGION_SIZE) + (fastFloat::mod(chunk.chunkID.y, REGION_SIZE) * REGION_SIZE) + fastFloat::mod(chunk.chunkID.z, REGION_SIZE);

				int index = regionContainer.getIndex(crntRegionID);

				if (index == -1)
					std::cout << "I am gonna cry now\n";

				Region &region = regionContainer.regions.at(index);

				region.regionID = crntRegionID;
				region.chunksInRegion[chunkIndex] = &chunk;
				region.isChunkNull[chunkIndex] = true;

				regionContainer.coordToIndexMap[regionContainer.coordsToKey(region.regionID)] = index;

				region.empty = false;
            }
            firstRun = 1;
        } else {
            for (int i = 0; i < (RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE); i++) {
            	ChunkDataContainer &chunk = worldContainer.chunks[i];
                std::array<int, 3> axisDistances {
                    chunk.chunkID.x - cameraChunk.x,
                    chunk.chunkID.y - cameraChunk.y,
                    chunk.chunkID.z - cameraChunk.z,
                };

                ChunkCoords crntRegionID;
				crntRegionID.x = floor(float(chunk.chunkID.x) / float(REGION_SIZE));
				crntRegionID.y = floor(float(chunk.chunkID.y) / float(REGION_SIZE));
				crntRegionID.z = floor(float(chunk.chunkID.z) / float(REGION_SIZE));

				int chunkIndex = (fastFloat::mod(chunk.chunkID.x, REGION_SIZE) * REGION_SIZE * REGION_SIZE) + (fastFloat::mod(chunk.chunkID.y, REGION_SIZE) * REGION_SIZE) + fastFloat::mod(chunk.chunkID.z, REGION_SIZE);

				int index = regionContainer.getIndex(crntRegionID);

                ChunkCoords newChunkID;
                bool x = false;
                if (!(axisDistances.at(0) < RENDER_DISTANCE && axisDistances.at(0) >= 0)) {
                    int m = fastFloat::mod(axisDistances.at(0), RENDER_DISTANCE);
                    newChunkID.x = cameraChunk.x + m;
                    x = true;
                } else {
                    newChunkID.x = chunk.chunkID.x;
                }
                if (!(axisDistances.at(1) < RENDER_DISTANCE && axisDistances.at(1) >= 0)) {
                    int m = fastFloat::mod(axisDistances.at(1), RENDER_DISTANCE);
                    newChunkID.y = cameraChunk.y + m;
                    x = true;
                } else {
                    newChunkID.y = chunk.chunkID.y;
                }
                if (!(axisDistances.at(2) < RENDER_DISTANCE && axisDistances.at(2) >= 0)) {
                    int m = fastFloat::mod(axisDistances.at(2), RENDER_DISTANCE);
                    newChunkID.z = cameraChunk.z + m;
                    x = true;
                } else {
                    newChunkID.z = chunk.chunkID.z;
                }
                if (x) {
                    worldContainer.coordToIndexMap.erase(worldContainer.coordsToKey(chunk.chunkID));

                    if (index != -1) {
                    	regionContainer.regions[index].isChunkNull[chunkIndex] = false;
                    	if (regionContainer.regions[index].isChunkNull == std::bitset<64>(0)) {
                    		regionContainer.coordToIndexMap.erase(regionContainer.coordsToKey(crntRegionID));
                    		regionContainer.regions[index].empty = true;
                    	}
                    }

                    chunk.chunkID = newChunkID;

                    worldContainer.coordToIndexMap[worldContainer.coordsToKey(chunk.chunkID)] = i;

                    chunk.unGeneratedChunk = 1;
                    
                    chunk.unCompiledChunk = 1;
                    worldContainer.chunks[i].meshSize = 0;
                    chunk.chunkData.clear();
                    chunk.renderlck = 1;
                    chunk.emptyChunk = 0;
                    chunk.inQueue = 0;
                    chunk.isPermeableCheckDone = false;
                    chunk.occlusionUnCulled = false;
                    chunk.redoRegionMesh = false;

                    crntRegionID.x = floor(float(chunk.chunkID.x) / float(REGION_SIZE));
					crntRegionID.y = floor(float(chunk.chunkID.y) / float(REGION_SIZE));
					crntRegionID.z = floor(float(chunk.chunkID.z) / float(REGION_SIZE));

					chunkIndex = (fastFloat::mod(chunk.chunkID.x, REGION_SIZE) * REGION_SIZE * REGION_SIZE) + (fastFloat::mod(chunk.chunkID.y, REGION_SIZE) * REGION_SIZE) + fastFloat::mod(chunk.chunkID.z, REGION_SIZE);

					index = regionContainer.getIndex(crntRegionID);

					if (index == -1)
						std::cout << "I am gonna cry now\n";

					Region &region = regionContainer.regions.at(index);

					region.regionID = crntRegionID;
					region.chunksInRegion[chunkIndex] = &chunk;
					region.isChunkNull[chunkIndex] = true;

					regionContainer.coordToIndexMap[regionContainer.coordsToKey(region.regionID)] = index;

					region.empty = false;
                }
            }
        }
        if (regionContainer.coordToIndexMap.size() > regionContainer.regions.size())
        	std::cout << "The hashmap for the regions probably has a very bad memory leak\n";

        cameraChunk.x = int(floor(camera.Position.x / CHUNK_SIZE));
        cameraChunk.y = int(floor(camera.Position.y / CHUNK_SIZE));
        cameraChunk.z = int(floor(camera.Position.z / CHUNK_SIZE));


        // const auto start = std::chrono::high_resolution_clock::now();
        genQueueGenerator.doBFS(cameraChunk);
        queueGenerator.doBFS(cameraChunk);

        for (int i = 0; i < (RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE); i++) {
            worldContainer.chunks.at(i).inBFSqueue = 0;
            worldContainer.chunks.at(i).occlusionUnCulled = queueGenerator.localOcclusionUnCulled.at(i);
            queueGenerator.localOcclusionUnCulled.at(i) = false;
        }
        // const auto end = std::chrono::high_resolution_clock::now();
 
        // const std::chrono::duration<double> diff = end - start;
        // std::cout << "BFS search: " << diff << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void ChunkProcessManager::buildChunks() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    while (run == 1) {
        if (organiselck == 0) {            
        	int index;

            bool iLetYouRun = false;
            
            if (chunkUpdateQueue.size() > 0) {
            	index = chunkUpdateQueue.front();
                if (worldContainer.chunks.at(index).unGeneratedChunk == false)
                    iLetYouRun = true;
                chunkUpdateQueue.pop();
            } else if (queueGenerator.chunkMeshingQueue[queueGenerator.crntMeshingQueue].size() > 0) {
                index = queueGenerator.chunkMeshingQueue[queueGenerator.crntMeshingQueue].front();
                if (worldContainer.chunks.at(index).unCompiledChunk == true && worldContainer.chunks.at(index).unGeneratedChunk == false)
                    iLetYouRun = true;
                queueGenerator.chunkMeshingQueue[queueGenerator.crntMeshingQueue].pop();
                worldContainer.chunks.at(index).inQueue = false;
            }

            ChunkDataContainer &chunk = worldContainer.chunks[index];
            
            if (iLetYouRun) {
                builder.chunkX = chunk.chunkID.x;
                builder.chunkY = chunk.chunkID.y;
                builder.chunkZ = chunk.chunkID.z;
                if ((chunk.unCompiledChunk == 1 || chunk.forUpdate == 1) && chunk.unGeneratedChunk == 0) {
                    chunk.vaolck = 1;
                    lighting.updateLight(chunk.chunkID);
                    // const auto start = std::chrono::high_resolution_clock::now();

                    builder.buildChunk(index);

	                // const auto end = std::chrono::high_resolution_clock::now();
	 
	                // const std::chrono::duration<double> diff = end - start;
	                // std::cout << "Generating a chunk: " << diff << "\n";

	                ChunkCoords crntRegionID;
					crntRegionID.x = floor(float(chunk.chunkID.x) / float(REGION_SIZE));
					crntRegionID.y = floor(float(chunk.chunkID.y) / float(REGION_SIZE));
					crntRegionID.z = floor(float(chunk.chunkID.z) / float(REGION_SIZE));

					int regionIndex = regionContainer.getIndex(crntRegionID);

					regionContainer.regions.at(regionIndex).shouldCompile = true;


                    worldContainer.chunks[index].vaolck = 0;
                } else {
                    worldContainer.chunks[index].vaolck = 1;
                }
            }
        }
    }
}

void ChunkProcessManager::generateChunks() {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    while (run == 1) {
        if (organiselck == 0) {
            int index = 0;
            for (int i = 0; i < worldContainer.chunks.size(); i++) {
                if (worldContainer.chunks[i].distance <= worldContainer.chunks[index].distance && worldContainer.chunks[i].unGeneratedChunk == 1) {
                    index = i;
                }
            }

            // Code like this will come in the future

			// bool iLetYouRun = false;
            
            // if (chunkGeneratingQueue.size() > 0) {
            //     index = chunkGeneratingQueue.front();
            //     //if (worldContainer.chunks.at(index).inGeneratorQueue) {
            //         iLetYouRun = true;
            //     //}
            //     chunkGeneratingQueue.pop();
            //     worldContainer.chunks.at(index).inGeneratorQueue = 0;
            // }

            // int index = 0;

            // bool iLetYouRun = false;

            // if (genQueueGenerator.chunkGeneratingQueue[genQueueGenerator.crntGeneratingQueue].size() > 0) {
            // 	index = genQueueGenerator.chunkGeneratingQueue[genQueueGenerator.crntGeneratingQueue].front();
            //     if (worldContainer.chunks.at(index).unGeneratedChunk == true)
            //         iLetYouRun = true;
            //     genQueueGenerator.chunkGeneratingQueue[genQueueGenerator.crntGeneratingQueue].pop();
            //     worldContainer.chunks.at(index).inGeneratorQueue = false;
            // }


            //std::cout << worldContainer.chunks[index].unGeneratedChunk <<  " Gen || Index: " << index << " || X Y Z: " << worldContainer.chunks[index].chunkID[0] << " " << worldContainer.chunks[index].chunkID[1] << " " << worldContainer.chunks[index].chunkID[2] << " || Distance: " << worldContainer.chunks[index].distance << "\n";
            if (/*iLetYouRun*/worldContainer.chunks.at(index).unGeneratedChunk == true) {

                generator.initChunk(worldContainer.chunks[index].chunkData);
                generator.generateChunk(worldContainer.chunks[index].chunkData, worldContainer.chunks[index].chunkID.x, worldContainer.chunks[index].chunkID.y, worldContainer.chunks[index].chunkID.z);
                
                worldContainer.chunks[index].unGeneratedChunk = 0;
                worldContainer.chunks[index].unCompiledChunk = 1;

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