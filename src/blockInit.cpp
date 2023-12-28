#include "chunkList.h"

BlockDefs::BlockDefs() {
    blocks[0].isSolid = 0;  blocks[0].castsAO = 0;
    blocks[2].isSolid = 1;  blocks[1].castsAO = 1;
    blocks[3].isSolid = 1;  blocks[2].castsAO = 1;
    blocks[4].isSolid = 0;  blocks[3].castsAO = 1;
}

BlockCoords::BlockCoords(int index) {
    x = index / (CHUNK_SIZE * CHUNK_SIZE);
    y = (index / CHUNK_SIZE) % CHUNK_SIZE;
    z = index % CHUNK_SIZE;
}

BlockCoords::BlockCoords(int _x, int _y, int _z) {
    x = _x;
    y = _y;
    z = _z;
}