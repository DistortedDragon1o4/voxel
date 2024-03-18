#include "chunkDataContainer.h"
#include "chunkList.h"

BuddyMemoryAllocator::BuddyMemoryAllocator(const unsigned int _totalMemoryBlockSize, const unsigned int numLayers) {
	totalMemoryBlockSize = _totalMemoryBlockSize;
	minimumMemoryBlockSize = _totalMemoryBlockSize >> numLayers;
	totalLayers = numLayers;

	memoryTree = std::vector<BuddyMemoryNode>((1 << (numLayers + 1)) - 1);

	memoryRegisterBuffer.create();
	memoryRegisterBuffer.allocate(sizeof(memoryRegister), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
	memoryRegisterBuffer.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	memoryRegisterBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);

	memoryBlock.create();
	memoryBlock.allocate(totalMemoryBlockSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
	memoryBlock.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	memoryBlock.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
}

bool BuddyMemoryAllocator::allocate(ChunkDataContainer &chunk) {
	if (chunk.mesh.size() > 0) {
		if (chunk.meshLivesOnGPU)
			deallocate(chunk);

		unsigned int crntBuddyBlockSize = std::max(unsigned(1 << (32 - __builtin_clz((chunk.mesh.size() + 1) * sizeof(unsigned int)))), minimumMemoryBlockSize);

		unsigned int crntLayer = (32 - __builtin_clz(totalMemoryBlockSize)) - (32 - __builtin_clz(crntBuddyBlockSize));

		unsigned int crntBuddyRegionBaseIndex = (1 << (crntLayer)) - 1;
		unsigned int crntBuddyRegionNodes = (1 << (crntLayer));

		for (int i = 0; i < crntBuddyRegionNodes; i++) {
			if (!memoryTree.at(crntBuddyRegionBaseIndex + i).hasData) {
				memoryTree.at(crntBuddyRegionBaseIndex + i).hasData = true;
				for (int j = 0; j < totalLayers - crntLayer; j++) {
					crntBuddyRegionBaseIndex = (crntBuddyRegionBaseIndex << 1) + 1;
					for (int k = (1 << (j + 1)) * i; k < (1 << (j + 1)) * (i + 1); k++)
						memoryTree.at(crntBuddyRegionBaseIndex + k).hasData = true;
				}
				crntBuddyRegionBaseIndex = (1 << crntLayer) - 1;
				for (int j = 0; j < crntLayer; j++) {
					crntBuddyRegionBaseIndex = crntBuddyRegionBaseIndex >> 1;
					unsigned int k = i >> (j + 1);
					memoryTree.at(crntBuddyRegionBaseIndex + k).hasData = true;
				}

				MemRegUnit thisUnit = {
					.chunkID = chunk.chunkID,
					.memoryIndex = i * crntBuddyBlockSize,
					.size = (unsigned int)(chunk.mesh.size() * sizeof(unsigned int))
				};

				memoryRegister.at(chunk.neighbouringChunkIndices[13]) = thisUnit;

				int magic = 738051;

				memcpy((char*)memoryBlock.persistentMappedPtr + (i * crntBuddyBlockSize), &magic, sizeof(unsigned int));
				memcpy((char*)memoryBlock.persistentMappedPtr + (i * crntBuddyBlockSize) + sizeof(unsigned int), &chunk.mesh[0], chunk.mesh.size() * sizeof(unsigned int));

				// std::cout << "Allocated memory for chunk index: " << chunk.neighbouringChunkIndices[13] << " of size " << chunk.mesh.size() * sizeof(unsigned int) << " bytes in buddy block of size " << crntBuddyBlockSize << " bytes at memory index " << memoryRegister.at(chunk.neighbouringChunkIndices[13]).memoryIndex << " on the buffer in the GPU. Allocation at node: (" << i << ", " << crntLayer << "). Location of the chunk is (" << chunk.chunkID.x << ", " << chunk.chunkID.y << ", " << chunk.chunkID.z << ")\n";

				memcpy((MemRegUnit*)memoryRegisterBuffer.persistentMappedPtr + chunk.neighbouringChunkIndices[13], &thisUnit, sizeof(MemRegUnit));

				chunk.meshLivesOnGPU = true;
				chunk.mesh.clear();

				chunk.reUploadMesh = false;

				return true;
			}
		}
		std::cout << "Uhh...I guess we ran out of memory?\n";
		return false;
	}
	chunk.reUploadMesh = false;
	return true;
}

void BuddyMemoryAllocator::deallocate(ChunkDataContainer &chunk) {
	if (!chunk.meshLivesOnGPU)
		return;

	unsigned int crntBuddyBlockSize = std::max(unsigned(1 << (32 - __builtin_clz(memoryRegister.at(chunk.neighbouringChunkIndices[13]).size + sizeof(unsigned int)))), minimumMemoryBlockSize);

	unsigned int crntLayer = (32 - __builtin_clz(totalMemoryBlockSize)) - (32 - __builtin_clz(crntBuddyBlockSize));

	unsigned int crntBuddyRegionBaseIndex = (1 << (crntLayer)) - 1;

	unsigned int crntBuddyRegionOffset = memoryRegister.at(chunk.neighbouringChunkIndices[13]).memoryIndex / crntBuddyBlockSize;

	memoryTree.at(crntBuddyRegionBaseIndex + crntBuddyRegionOffset).hasData = false;
	for (int j = 0; j < totalLayers - crntLayer; j++) {
		crntBuddyRegionBaseIndex = (crntBuddyRegionBaseIndex << 1) + 1;
		for (int k = (1 << (j + 1)) * crntBuddyRegionOffset; k < (1 << (j + 1)) * (crntBuddyRegionOffset + 1); k++)
			memoryTree.at(crntBuddyRegionBaseIndex + k).hasData = false;
	}
	crntBuddyRegionBaseIndex = (1 << crntLayer) - 1;
	for (int j = 0; j < crntLayer; j++) {
		unsigned int i = (crntBuddyRegionOffset >> j) - (((crntBuddyRegionOffset >> j) % 2) * 2) + 1;
		if (!memoryTree.at(crntBuddyRegionBaseIndex + i).hasData) {
			crntBuddyRegionBaseIndex = crntBuddyRegionBaseIndex >> 1;
			unsigned int k = crntBuddyRegionOffset >> (j + 1);
			memoryTree.at(crntBuddyRegionBaseIndex + k).hasData = false;
		} else {
			break;
		}
	}

	MemRegUnit thisUnit = {
		.chunkID = chunk.chunkID,
		.memoryIndex = 0,
		.size = 0
	};

	memoryRegister.at(chunk.neighbouringChunkIndices[13]) = thisUnit;

	memcpy((MemRegUnit*)memoryRegisterBuffer.persistentMappedPtr + chunk.neighbouringChunkIndices[13], &thisUnit, sizeof(MemRegUnit));

	// std::cout << "Deallocated memory for chunk index: " << chunk.neighbouringChunkIndices[13] << " of size on the buffer in the GPU. Allocation at node: (" << crntBuddyRegionOffset << ", " << crntLayer << ")\n";

	chunk.meshLivesOnGPU = false;
}