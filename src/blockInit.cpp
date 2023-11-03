#include "../include/chunkList.h"

void ChunkList::blockInit() {
    blocks[0].isSolid = 0;  blocks[0].castsAO = 0;
    blocks[2].isSolid = 1;  blocks[1].castsAO = 1;
    blocks[3].isSolid = 1;  blocks[2].castsAO = 1;
    blocks[4].isSolid = 0;  blocks[3].castsAO = 1;
}