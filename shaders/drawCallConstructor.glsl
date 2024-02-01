#version 460 core

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

int CHUNK_SIZE = 16;

struct DrawCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

struct DrawCommandContainer {
	DrawCommand drawCommands[64];
};

layout (binding = 0, std430) buffer DrawCommandBufferData {
 	DrawCommandContainer drawCommandContainer[];
} drawCommandBufferData;

layout (binding = 4, std430) buffer AlternateDrawCommandBufferData {
 	DrawCommandContainer drawCommandContainer[];
} alternateDrawCommandBufferData;

layout (binding = 1, std430) buffer ChunkViewableBufferData {
	uint data[];
} chunkViewableBufferData;

void main() {
	uint regionIndex = (gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z) + (gl_WorkGroupID.y * gl_NumWorkGroups.z) + gl_WorkGroupID.z;
	uint chunkIndex = gl_LocalInvocationIndex;

	uint crntIndex = (chunkViewableBufferData.data[(64 * regionIndex) + chunkIndex] >> 2) - 1;

	if (((chunkViewableBufferData.data[(64 * regionIndex) + chunkIndex] >> 1) & 1) == 1) {
		alternateDrawCommandBufferData.drawCommandContainer[regionIndex].drawCommands[crntIndex] = drawCommandBufferData.drawCommandContainer[regionIndex].drawCommands[chunkIndex];
		alternateDrawCommandBufferData.drawCommandContainer[regionIndex].drawCommands[crntIndex].instanceCount = 1;
		alternateDrawCommandBufferData.drawCommandContainer[regionIndex].drawCommands[crntIndex].baseInstance = (chunkIndex << 26) | (0x3ffffff & alternateDrawCommandBufferData.drawCommandContainer[regionIndex].drawCommands[crntIndex].baseInstance);
	}
}