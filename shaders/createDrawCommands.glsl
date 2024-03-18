#version 460 core

layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

struct DrawCommand {
    uint count;
    uint instanceCount;
    uint baseVertex;
    uint baseInstance;
};

struct MemRegUnit {
	int x;
	int y;
	int z;
	int memoryIndex;
	int size;							// In bytes (size of the mesh)
};

layout (binding = 0, std430) buffer DrawCommandBufferData {
 	DrawCommand command[];
} drawCommandBufferData;

layout (binding = 1, std430) buffer ChunkViewableBufferData {
	uint drawCount;
	uint data[];
} chunkViewableBufferData;

layout (binding = 2, std430) buffer MemoryRegister {
 	MemRegUnit unit[];
} memoryRegister;

void main() {
	uint chunkIndex = (gl_WorkGroupID.x * 8) + gl_LocalInvocationID.x;

	int crntIndex = -1;
	if (chunkViewableBufferData.data[chunkIndex] == 1 && memoryRegister.unit[chunkIndex].size > 0)
		crntIndex = int(atomicAdd(chunkViewableBufferData.drawCount, 1));

	if (crntIndex != -1) {
		DrawCommand crntCommand;
		crntCommand.count = 6 * (memoryRegister.unit[chunkIndex].size / 32);
		crntCommand.instanceCount = 1;
		crntCommand.baseVertex = 0;
		crntCommand.baseInstance = chunkIndex;

		drawCommandBufferData.command[crntIndex] = crntCommand;
	}
}