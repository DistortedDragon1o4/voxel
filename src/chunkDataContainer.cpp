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

short ChunkData::blockAtCoords(const BlockCoords coords, const int LODlevel) {
	if (LODlevel == 0)
		return blockAtCoords(coords.x, coords.y, coords.z);
	if (LODlevel >= lodLevel)
		for (int i = 0; i < (1 << (3 * LODlevel)); i++) 
			if (blockAtCoords(((coords.x << LODlevel) + (i >> (1 << (2 * LODlevel)))), ((coords.y << LODlevel)) + ((i >> (1 << (LODlevel))) & ((1 << (LODlevel)) - 1)), ((coords.z << LODlevel)) + (i & ((1 << (LODlevel)) - 1))) == 0)
				return 0;
	return blockAtCoords(coords.x * (1 << LODlevel), coords.y * (1 << LODlevel), coords.z * (1 << LODlevel));
}

short ChunkData::blockAtCoords(const int _x, const int _y, const int _z, const int LODlevel) {
	if (LODlevel == 0)
		return blockAtCoords(_x, _y, _z);
	if (LODlevel >= lodLevel)
		for (int i = 0; i < (1 << (3 * LODlevel)); i++) 
			if (blockAtCoords(((_x << LODlevel) + (i >> (1 << (2 * LODlevel)))), ((_y << LODlevel)) + ((i >> (1 << (LODlevel))) & ((1 << (LODlevel)) - 1)), ((_z << LODlevel)) + (i & ((1 << (LODlevel)) - 1))) == 0)
				return 0;
	return blockAtCoords(_x * (1 << LODlevel), _y * (1 << LODlevel), _z * (1 << LODlevel));
}


















void ChunkLightContainer::compress(std::vector<unsigned int> &compressedData) {
	compressedData.push_back(0);
	if (isEmpty) {
		compressedData[0] = 1;
		compressedData.push_back(0);
		return;
	}

	unsigned int prevLightVal = 0;
	bool firstRun = true;
	for (auto &i : data) {
		if (firstRun) {
			firstRun = false;
			prevLightVal = i;
		} else {
			if (prevLightVal != i)
				goto skip;
		}
	}

	compressedData[0] = 1;
	compressedData.push_back(prevLightVal);
	return;

	skip:
	compressedData.insert(compressedData.end(), data.begin(), data.end());
}