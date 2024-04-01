#include "chunkDataContainer.h"
#include "coordinateContainers.h"

short ChunkData::blockAtCoords(const BlockCoords coords) {
	if (isSingleBlock)
		return theBlock;
	if (layerData[coords.y].isSingleBlock)
		return layerData[coords.y].theBlock;
	return layerData[coords.y].blockData[(coords.x * CHUNK_SIZE) + coords.z];
}

short ChunkData::blockAtCoords(const int _x, const int _y, const int _z) {
	if (isSingleBlock)
		return theBlock;
	if (layerData[_y].isSingleBlock)
		return layerData[_y].theBlock;
	return layerData[_y].blockData[(_x * CHUNK_SIZE) + _z];
}

void Layer::setBlockAtCoords(const int _x, const int _z, const short block) {
	if (isSingleBlock) {
		for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
			blockData.push_back(theBlock);
		blockData[(_x * CHUNK_SIZE) + _z] = block;
		isSingleBlock = false;
		theBlock = 0;
	} else {
		blockData[(_x * CHUNK_SIZE) + _z] = block;
		compressLayer();
	}
}

void ChunkData::setBlockAtCoords(const BlockCoords coords, const short block) {
	if (isSingleBlock) {
		for (int i = 0; i < CHUNK_SIZE; i++)
			layerData.emplace_back(theBlock);
		layerData[coords.y].setBlockAtCoords(coords.x, coords.z, block);
		isSingleBlock = false;
		theBlock = 0;
	} else {
		layerData[coords.y].setBlockAtCoords(coords.x, coords.z, block);
		compressChunk();
	}
}

void ChunkData::setBlockAtCoords(const int _x, const int _y, const int _z, const short block) {
	if (isSingleBlock) {
		for (int i = 0; i < CHUNK_SIZE; i++)
			layerData.emplace_back(theBlock);
		layerData[_y].setBlockAtCoords(_x, _z, block);
		isSingleBlock = false;
		theBlock = 0;
	} else {
		layerData[_y].setBlockAtCoords(_x, _z, block);
		compressChunk();
	}
}

void Layer::compressLayer() {
	if (isSingleBlock)
		return;

	short lastBlock = blockData[0];
	for (int i = 1; i < CHUNK_SIZE * CHUNK_SIZE; i++) {
		if (lastBlock != blockData[i])
			return;
	}
	isSingleBlock = true;
	theBlock = lastBlock;
	blockData.clear();
}

void ChunkData::compressChunk() {
	if (isSingleBlock || layerData.size() == 0)
		return;

	if (!layerData[0].isSingleBlock)
		return;

	short lastBlock = layerData[0].theBlock;
	for (int i = 1; i < CHUNK_SIZE; i++) {
		if (lastBlock != layerData[i].theBlock)
			return;
	}
	isSingleBlock = true;
	theBlock = lastBlock;
	layerData.clear();
}

void ChunkData::clear() {
	layerData.clear();
	theBlock = 0;
	isSingleBlock = true;
}

void ChunkData::raw(std::vector<short> &rawBlockData) {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		Layer crntLayer;
		for (int j = 0; j < CHUNK_SIZE * CHUNK_SIZE; j++) {
			crntLayer.blockData[j] = rawBlockData[((j / CHUNK_SIZE) * CHUNK_SIZE * CHUNK_SIZE) + (i * CHUNK_SIZE) + (j % CHUNK_SIZE)];
		}
		crntLayer.isSingleBlock = false;
		crntLayer.compressLayer();
		layerData.push_back(crntLayer);
	}
	isSingleBlock = false;
	compressChunk();
}