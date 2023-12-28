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

struct ChunkProperties {
    ivec3 chunkID;
    int index;
};

struct RegionDataContainer {
    ChunkProperties chunkProperties[64];
};

layout (binding = 0, std430) buffer DrawCommandBufferData {
 	DrawCommandContainer drawCommandContainer[];
} drawCommandBufferData;

layout (binding = 2, std430) buffer RegionPropertiesBufferData {
 	RegionDataContainer regionData[];
} regionPropertiesBufferData;

layout (binding = 1, std430) buffer ChunkViewableBufferData {
	uint data[];
} chunkViewableBufferData;

uniform float cosineModifiedHalfFOV;

uniform vec3 camPos;
uniform vec3 camDir;

uniform float frustumOffset;

uint isInsideViewFrustum(ivec3 chunkID) {
	vec3 chunkBaseCoords = chunkID * 16.0;
	vec3 newCamPos = vec3(camPos.x, camPos.y, camPos.z) - (frustumOffset * camDir);

	for (int i = 0; i < CHUNK_SIZE + 1; i += CHUNK_SIZE) {
		for (int j = 0; j < CHUNK_SIZE + 1; j += CHUNK_SIZE) {
			for (int k = 0; k < CHUNK_SIZE + 1; k += CHUNK_SIZE) {
				vec3 crntVertCoords = vec3(chunkBaseCoords.x + i, chunkBaseCoords.y + j, chunkBaseCoords.z + k);
				double dot = dot(normalize(crntVertCoords - newCamPos), camDir);
				if (dot >= cosineModifiedHalfFOV)
					return 1;
			}
		}
	}
	return 0;
}

void main() {
	uint regionIndex = (gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z) + (gl_WorkGroupID.y * gl_NumWorkGroups.z) + gl_WorkGroupID.z;
	uint chunkIndex = gl_LocalInvocationIndex;

	ivec3 chunkID = regionPropertiesBufferData.regionData[regionIndex].chunkProperties[chunkIndex].chunkID;

	drawCommandBufferData.drawCommandContainer[regionIndex].drawCommands[chunkIndex].instanceCount = (chunkViewableBufferData.data[(64 * regionIndex) + chunkIndex] & 1) & isInsideViewFrustum(chunkID);

	chunkViewableBufferData.data[(64 * regionIndex) + chunkIndex] = uint((chunkViewableBufferData.data[(64 * regionIndex) + chunkIndex] & isInsideViewFrustum(chunkID)) << 1) + chunkViewableBufferData.data[(64 * regionIndex) + chunkIndex];
}