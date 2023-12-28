#ifndef CONTAINERS_CLASS_H
#define CONTAINERS_CLASS_H

#include <array>

struct ChunkCoords {
    int x;
    int y;
    int z;
};

struct BlockCoords {
    BlockCoords(int index);
    BlockCoords(int _x, int _y, int _z);
    int x;
    int y;
    int z;
};

struct DrawCommand {
    unsigned int count = 0;
    unsigned int instanceCount = 0;
    unsigned int firstIndex = 0;
    unsigned int baseVertex = 0;
    unsigned int baseInstance = 0;
};

struct DrawCommandContainer {
    std::array<DrawCommand, 64> drawCommands;
};

struct ChunkProperties {
    ChunkCoords chunkID;
    int index;
};

struct RegionDataContainer {
    std::array<ChunkProperties, 64> chunkProperties;
};

#endif