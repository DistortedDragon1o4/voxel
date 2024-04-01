#pragma once

#include <array>
#include <functional>

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

struct RGB {
    RGB(const unsigned int colour) {r = (colour >> 16) & 0xff; g = (colour >> 8) & 0xff; b = colour & 0xff;};
    RGB(unsigned int _r, unsigned int _g, unsigned int _b) {r = _r; g = _g; b = _b;};
    unsigned int toInt() {return (r << 16) + (g << 8) + b;};

    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct LightUpdateInstruction {
    bool propagationType;   // true means propagate
    char propagateChannel;
    BlockCoords coords;
    unsigned char value;
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