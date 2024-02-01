#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (binding = 1, std430) buffer ChunkViewableBufferData {
	uint data[];
} chunkViewableBufferData;

void main() {
	uint regionIndex = (gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z) + (gl_WorkGroupID.y * gl_NumWorkGroups.z) + gl_WorkGroupID.z;
	
	uint accumulator = 0;
	for (int i = 0; i < 64; i++) {
		accumulator += (chunkViewableBufferData.data[(64 * regionIndex) + i] >> 1) & 1;
		chunkViewableBufferData.data[(64 * regionIndex) + i] |= accumulator << 2;
	}
}