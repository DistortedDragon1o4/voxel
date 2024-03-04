#include "chunkDataContainer.h"
#include "coordinateContainers.h"

short ChunkDataContainer::blockAtCoords(const BlockCoords coords) {
	if (isSingleBlock)
		return theBlock;
	return chunkData[(coords.x * CHUNK_SIZE * CHUNK_SIZE) + (coords.y * CHUNK_SIZE) + coords.z];
}

short ChunkDataContainer::blockAtCoords(const int _x, const int _y, const int _z) {
	if (isSingleBlock)
		return theBlock;
	return chunkData[(_x * CHUNK_SIZE * CHUNK_SIZE) + (_y * CHUNK_SIZE) + _z];
}

void ChunkDataContainer::setBlockAtCoords(const BlockCoords coords, const short block) {
	if (isSingleBlock) {
		for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++)
			chunkData.push_back(theBlock);
		chunkData[(coords.x * CHUNK_SIZE * CHUNK_SIZE) + (coords.y * CHUNK_SIZE) + coords.z] = block;
		isSingleBlock = false;
		theBlock = 0;
	} else {
		chunkData[(coords.x * CHUNK_SIZE * CHUNK_SIZE) + (coords.y * CHUNK_SIZE) + coords.z] = block;
		compressChunk();
	}
}

void ChunkDataContainer::setBlockAtCoords(const int _x, const int _y, const int _z, const short block) {
	if (isSingleBlock) {
		for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++)
			chunkData.push_back(theBlock);
		chunkData[(_x * CHUNK_SIZE * CHUNK_SIZE) + (_y * CHUNK_SIZE) + _z] = block;
		isSingleBlock = false;
		theBlock = 0;
	} else {
		chunkData[(_x * CHUNK_SIZE * CHUNK_SIZE) + (_y * CHUNK_SIZE) + _z] = block;
		compressChunk();
	}
}

void ChunkDataContainer::compressChunk() {
	if (isSingleBlock || unGeneratedChunk)
		return;

	short lastBlock = chunkData[0];
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
		if (lastBlock != chunkData[i])
			return;
		lastBlock = chunkData[i];
	}
	isSingleBlock = true;
	theBlock = lastBlock;
	chunkData.clear();
}