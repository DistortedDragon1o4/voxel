#version 460 core

layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

float CHUNK_SIZE = 32.0;

struct MemRegUnit {
	int x;
	int y;
	int z;
	int memoryIndex;
	int size;							// In bytes (size of the mesh)
};
layout (binding = 1, std430) buffer ChunkViewableBufferData {
	uint drawCount;
	uint data[];
} chunkViewableBufferData;

layout (binding = 2, std430) buffer MemoryRegister {
 	MemRegUnit unit[];
} memoryRegister;

uniform vec3 camPos;
uniform mat4 cameraMatrix;

uint isInsideViewFrustum(ivec3 chunkID) {
	vec3 chunkBaseCoords = chunkID * CHUNK_SIZE;

	for (int i = 0; i < 8; i++) {
		vec3 crntCoordsOffset = vec3(float(i >> 2), float((i >> 1) & 1), float(i & 1));
		vec3 crntVertCoords = chunkBaseCoords + (CHUNK_SIZE * crntCoordsOffset) - camPos;

		vec4 clipSpacePos = cameraMatrix * vec4(crntVertCoords, 1.0);

		if ((abs(clipSpacePos.x) <= clipSpacePos.w && abs(clipSpacePos.y) <= clipSpacePos.w && abs(clipSpacePos.z) <= clipSpacePos.w) || (distance(crntVertCoords, vec3(0.0, 0.0, 0.0)) <= CHUNK_SIZE))
			return 1;
	}
	return 0;
}

void main() {
	uint chunkIndex = (gl_WorkGroupID.x * 8) + gl_LocalInvocationID.x;

	ivec3 chunkID = ivec3(memoryRegister.unit[chunkIndex].x, memoryRegister.unit[chunkIndex].y, memoryRegister.unit[chunkIndex].z);

	chunkViewableBufferData.data[chunkIndex] = uint(chunkViewableBufferData.data[chunkIndex] & isInsideViewFrustum(chunkID));

	chunkViewableBufferData.drawCount = 0;
}