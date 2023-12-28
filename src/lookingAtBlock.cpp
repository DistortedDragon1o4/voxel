#include "../include/chunkList.h"
#include "chunkDataContainer.h"
#include "chunkGenerator.h"
#include "fastFloat.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"

RayCastReturn RayCaster::rayCastTillBlock(const glm::dvec3 ray, const glm::dvec3 position, const double limit) {
    glm::dvec3 newPosX;
    glm::dvec3 newPosY;
    glm::dvec3 newPosZ;
    glm::dvec3 crntRayPosInt = glm::dvec3(floor(position));
    glm::dvec3 crntRayPos = position;
    glm::dvec3 prevRayPosInt = position;
    int prevIndex = -1;
    int iter = 0;
    RayCastReturn casterResults;
    while (glm::distance(crntRayPos, position) <= limit) {
        // check when the ray collides with the next x plane
        int x;
        if (ray.x > 0)
            x = floor(crntRayPos.x) + 1;
        else if (ray.x < 0)
            x = ceil(crntRayPos.x) - 1;
        else if (ray.x == 0)
            x = 2147483647;
        // storing the appropriate values
        if (x != 2147483647) {
            newPosX.x = x;
            newPosX.y = (ray.y * ((x - crntRayPos.x) / ray.x)) + crntRayPos.y;
            newPosX.z = (ray.z * ((x - crntRayPos.x) / ray.x)) + crntRayPos.z;
        } else {
            newPosX = glm::dvec3(2147483647, 2147483647, 2147483647);
        }

        // check when the ray collides with the next y plane
        int y;
        if (ray.y > 0)
            y = floor(crntRayPos.y) + 1;
        else if (ray.y < 0)
            y = ceil(crntRayPos.y) - 1;
        else if (ray.y == 0)
            y = 2147483647;
        // storing the appropriate values
        if (y != 2147483647) {
            newPosY.x = (ray.x * ((y - crntRayPos.y) / ray.y)) + crntRayPos.x;
            newPosY.y = y;
            newPosY.z = (ray.z * ((y - crntRayPos.y) / ray.y)) + crntRayPos.z;
        } else {
            newPosY = glm::dvec3(2147483647, 2147483647, 2147483647);
        }

        // check when the ray collides with the next z plane
        int z;
        if (ray.z > 0)
            z = floor(crntRayPos.z) + 1;
        else if (ray.z < 0)
            z = ceil(crntRayPos.z) - 1;
        else if (ray.z == 0)
            z = 2147483647;
        // storing the appropriate values
        if (z != 2147483647) {
            newPosZ.x = (ray.x * ((z - crntRayPos.z) / ray.z)) + crntRayPos.x;
            newPosZ.y = (ray.y * ((z - crntRayPos.z) / ray.z)) + crntRayPos.y;
            newPosZ.z = z;
        } else {
            newPosZ = glm::dvec3(2147483647, 2147483647, 2147483647);
        }


        double min = std::min(std::min(glm::distance(newPosX, crntRayPos), glm::distance(newPosY, crntRayPos)), glm::distance(newPosZ, crntRayPos));

        if (min == glm::distance(newPosX, crntRayPos)) {
            if (newPosX != crntRayPos)
                crntRayPos = newPosX;
        }
        if (min == glm::distance(newPosY, crntRayPos)) {
            if (newPosY != crntRayPos)
                crntRayPos = newPosY;
        }
        if (min == glm::distance(newPosZ, crntRayPos)) {
            if (newPosZ != crntRayPos)
                crntRayPos = newPosZ;
        }

        prevRayPosInt = crntRayPosInt;

        if (ray.x < 0)
            crntRayPosInt.x = ceil(crntRayPos.x) - 1;
        if (ray.x >= 0)
            crntRayPosInt.x = floor(crntRayPos.x);

        if (ray.y < 0)
            crntRayPosInt.y = ceil(crntRayPos.y) - 1;
        if (ray.y >= 0)
            crntRayPosInt.y = floor(crntRayPos.y);

        if (ray.z < 0)
            crntRayPosInt.z = ceil(crntRayPos.z) - 1;
        if (ray.z >= 0)
            crntRayPosInt.z = floor(crntRayPos.z);


        //std::cout << iter << "-" << crntRayPos.x << " " << crntRayPos.y << " " << crntRayPos.z << "\n";

        ChunkCoords chunkCoords;
        chunkCoords.x = int(floor(crntRayPosInt.x / CHUNK_SIZE));
        chunkCoords.y = int(floor(crntRayPosInt.y / CHUNK_SIZE));
        chunkCoords.z = int(floor(crntRayPosInt.z / CHUNK_SIZE));

        int index;
        try {
            index = worldContainer.coordToIndexMap.at(worldContainer.coordsToKey(chunkCoords));
        } catch (const std::out_of_range& oor) {
            casterResults.blockPos = glm::ivec3(2147483647, 2147483647, 2147483647);
            casterResults.prevBlockPos = glm::ivec3(2147483647, 2147483647, 2147483647);
            casterResults.blockPosIndex = -1;
            casterResults.prevBlockPosIndex = -1;
            break;
        }

        std::array<int, 3> coords {
            int(floor(crntRayPosInt.x)) - (CHUNK_SIZE * chunkCoords.x),
            int(floor(crntRayPosInt.y)) - (CHUNK_SIZE * chunkCoords.y),
            int(floor(crntRayPosInt.z)) - (CHUNK_SIZE * chunkCoords.z)
        };

        if(worldContainer.chunks.at(index).chunkData.size() == CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE && worldContainer.chunks.at(index).chunkData.at((coords.at(0) * CHUNK_SIZE * CHUNK_SIZE) + (coords.at(1) * CHUNK_SIZE) + coords.at(2)) > 0) {
            casterResults.blockPos = glm::ivec3(crntRayPosInt);
            casterResults.prevBlockPos = glm::ivec3(prevRayPosInt);
            casterResults.blockPosIndex = index;
            casterResults.prevBlockPosIndex = prevIndex;
            break;
        } else {
            casterResults.blockPos = glm::ivec3(2147483647, 2147483647, 2147483647);
            casterResults.prevBlockPos = glm::ivec3(2147483647, 2147483647, 2147483647);
            casterResults.blockPosIndex = -1;
            casterResults.prevBlockPosIndex = -1;
        }
        prevIndex = index;
        iter++;
    }
    return casterResults;
}

void PlayerChunkInterface::breakBlock() {
    ChunkCoords chunkCoords;
    chunkCoords.x = int(floor(float(highlightCursor.crntLookingAtBlock.blockPos.x) / CHUNK_SIZE));
    chunkCoords.y = int(floor(float(highlightCursor.crntLookingAtBlock.blockPos.y) / CHUNK_SIZE));
    chunkCoords.z = int(floor(float(highlightCursor.crntLookingAtBlock.blockPos.z) / CHUNK_SIZE));
    BlockCoords coords(fastFloat::mod(highlightCursor.crntLookingAtBlock.blockPos.x, CHUNK_SIZE), fastFloat::mod(highlightCursor.crntLookingAtBlock.blockPos.y, CHUNK_SIZE), fastFloat::mod(highlightCursor.crntLookingAtBlock.blockPos.z, CHUNK_SIZE));
    if (highlightCursor.crntLookingAtBlock.blockPosIndex >= 0) {
        worldContainer.chunks.at(highlightCursor.crntLookingAtBlock.blockPosIndex).chunkData.at((coords.x * CHUNK_SIZE * CHUNK_SIZE) + (coords.y * CHUNK_SIZE) + coords.z) = 0;
        processManager.updateChunk(chunkCoords, 1);
    }
}

void PlayerChunkInterface::placeBlock() {
    ChunkCoords chunkCoords;
    chunkCoords.x = int(floor(float(highlightCursor.crntLookingAtBlock.prevBlockPos.x) / CHUNK_SIZE));
    chunkCoords.y = int(floor(float(highlightCursor.crntLookingAtBlock.prevBlockPos.y) / CHUNK_SIZE));
    chunkCoords.z = int(floor(float(highlightCursor.crntLookingAtBlock.prevBlockPos.z) / CHUNK_SIZE));
    BlockCoords coords(fastFloat::mod(highlightCursor.crntLookingAtBlock.prevBlockPos.x, CHUNK_SIZE), fastFloat::mod(highlightCursor.crntLookingAtBlock.prevBlockPos.y, CHUNK_SIZE), fastFloat::mod(highlightCursor.crntLookingAtBlock.prevBlockPos.z, CHUNK_SIZE));
    if (highlightCursor.crntLookingAtBlock.prevBlockPosIndex >= 0) {
        worldContainer.chunks.at(highlightCursor.crntLookingAtBlock.prevBlockPosIndex).chunkData.at((coords.x * CHUNK_SIZE * CHUNK_SIZE) + (coords.y * CHUNK_SIZE) + coords.z) = currentBlock;
        processManager.updateChunk(chunkCoords, 1);
    }
}




void HighlightCursor::positionCursor() {
    crntLookingAtBlock = rayCaster.rayCastTillBlock(camera.Orientation, camera.Position, 10.0);
}

void HighlightCursor::createHighlightVAO() {
    highlightVAO.Bind();

    VBO VBO1;
    EBO EBO1;
    
    VBO1.Gen(highlightCube);
    EBO1.Gen(highlightEBO);

    highlightVAO.LinkAttribPointer(VBO1, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

    highlightVAO.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    VBO1.Delete();
    EBO1.Delete();
}

void HighlightCursor::renderCursor() {
    shaderProgramHighlight.Activate();

    camera.matrix(90.0, 0.001, 512.0, shaderProgramHighlight, "cameraMatrix");
    glUniform3fv(locPos, 1, &glm::vec3(crntLookingAtBlock.blockPos)[0]);
    glUniform3fv(locCamPos, 1, &glm::vec3(camera.Position)[0]);

    highlightVAO.Bind();
    glDrawElements(GL_LINE_STRIP, EBOsize, GL_UNSIGNED_INT, 0);
    highlightVAO.Unbind();
}