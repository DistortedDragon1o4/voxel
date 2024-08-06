#include "chunkList.h"

long WorldContainer::coordsToKey(const ChunkCoords coords) {
    long result = (fastFloat::mod(coords.x, 1024) * 1024 * 1024) + (fastFloat::mod(coords.y, 1024) * 1024) + fastFloat::mod(coords.z, 1024);
    return result;
}

int WorldContainer::getIndex(const ChunkCoords chunkCoord) {
    int index;
    try {
        index = coordToIndexMap.at(coordsToKey(chunkCoord));
    } catch (const std::out_of_range& oor) {
        //std::cerr << "Out of Range error: " << oor.what() << "\n";
        return -1;
    }
    return index;
}

int WorldContainer::getIndex(const int chunkCoordX, const int chunkCoordY, const int chunkCoordZ) {
    ChunkCoords chunkCoord;
    chunkCoord.x = chunkCoordX;
    chunkCoord.y = chunkCoordY;
    chunkCoord.z = chunkCoordZ;
    int index;
    try {
        index = coordToIndexMap.at(coordsToKey(chunkCoord));
    } catch (const std::out_of_range& oor) {
        //std::cerr << "Out of Range error: " << oor.what() << "\n";
        return -1;
    }
    return index;
}