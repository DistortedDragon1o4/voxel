#version 460 core

layout (location = 0) in int dataBlock1;
layout (location = 1) in int dataBlock2;
// layout (location = 2) in int regionIndex;

layout (binding = 3, std430) buffer SSBOData {
 	float lightData[];
} ssboData;

struct ChunkProperties {
    ivec3 chunkID;
    int index;
};

struct RegionDataContainer {
    ChunkProperties chunkProperties[64];
};

layout (binding = 2, std430) buffer RegionPropertiesBufferData {
 	RegionDataContainer regionData[];
} regionPropertiesBufferData;

// struct ChunkProperties {
// 	ivec3 chunkID;
// 	int index;
// };

// struct RegionProperties {
// 	// Size is 64 because there are 64 chunks in a region
// 	ChunkProperties chunkProperties[64];
// };

// layout (binding = 5, std430) buffer ChunkPropertiesBufferData {
//  	RegionProperties regionProperties[];
// } chunkPropertiesBufferData;


out vec2 texCoord;

// out float ambientOcc;

out float blockID;
out vec3 normal;
out float camDistance;
out float blockLight;

out vec4 ambientOcc;

out vec2 ambientOccPos;

out vec3 chunkIDx;


uniform mat4 cameraMatrix;
uniform vec3 camPos;


float CHUNK_SIZE = 16.0;

vec3 normalArr[6] = {
	vec3(0.0f, -1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(-1.0f, 0.0f, 0.0f),
	vec3(1.0f, 0.0f, 0.0f),
	vec3(0.0f, 0.0f, -1.0f),
	vec3(0.0f, 0.0f, 1.0f),
};

ivec2 ambientOccArr[4] = {
	ivec2(0, 0),
	ivec2(1, 0),
	ivec2(1, 1),
	ivec2(0, 1),
};

float ambientOccMap[4] = {
 	0.0, 0.7, 0.7, 1.0
};

void main() {

	//ivec3 chunkID = chunkPropertiesBufferData.regionProperties[regionIndex].chunkProperties[gl_DrawID].chunkID;
	//int index = chunkPropertiesBufferData.regionProperties[regionIndex].chunkProperties[gl_DrawID].index;

	ivec3 chunkID = regionPropertiesBufferData.regionData[gl_BaseInstance].chunkProperties[gl_DrawID].chunkID;
	chunkIDx = chunkID;
	int index = regionPropertiesBufferData.regionData[gl_BaseInstance].chunkProperties[gl_DrawID].index;

	// Stuff left to do to fix precision issues

	const int coordMask = 0xff;
	ivec3 coords = ivec3(((dataBlock1 & (coordMask << 16)) >> 16), ((dataBlock1 & (coordMask << 8)) >> 8), (dataBlock1 & coordMask));
	vec3 mesh = vec3(float(coords.x / 8.0) + (chunkID.x * CHUNK_SIZE), float(coords.y / 8.0) + (chunkID.y * CHUNK_SIZE), float(coords.z / 8.0) + (chunkID.z * CHUNK_SIZE));
	mesh = mesh - camPos;
	gl_Position = vec4(cameraMatrix * vec4(mesh, 1.0));

	// ivec3 quadVertices[4];

	// quadVertices[gl_VertexID] = coords;

	// if (gl_VertexID % 4 == 0) {
	//   quadVertices[1] = ivec3(coords.)
	// }


	camDistance = distance(vec3(mesh), camPos);

	const int texMask = 0xff;
	int texMap = (dataBlock1 & (texMask << 24)) >> 24;
	ivec2 texInt = ivec2(texMap >> 4, texMap & 0xf);
	texCoord = vec2(float(texInt.x / 8.0), float(texInt.y / 8.0));

	blockID = int(dataBlock2 >> 11);

	// ambientOcc = float(int(dataBlock2 & 1) / 10.0);

	const int normalMask = 0xe;
	int normalMap = (dataBlock2 >> 8) & normalMask;
	normal = normalArr[normalMap];

	vec4 ambientOccLocal;
	ambientOccLocal.x = ambientOccMap[int((dataBlock2 >> 6) & 3)];
	ambientOccLocal.y = ambientOccMap[int((dataBlock2 >> 4) & 3)];
	ambientOccLocal.z = ambientOccMap[int((dataBlock2 >> 2) & 3)];
	ambientOccLocal.w = ambientOccMap[int(dataBlock2 & 3)];

	ambientOcc = ambientOccLocal;

	/*if (ambientOcc != vec4(1.0)) {
	if (ambientOccLocal.y == 1.0 && ambientOccLocal.w == 1.0) {
	  if (ambientOccLocal.x == 1.0)
	    ambientOcc.x = 1.41421356237;
	  if (ambientOccLocal.z == 1.0)
	    ambientOcc.z = 1.41421356237;
	}
	if (ambientOccLocal.z == 1.0 && ambientOccLocal.x == 1.0) {
	  if (ambientOccLocal.y == 1.0)
	    ambientOcc.y = 1.41421356237;
	  if (ambientOccLocal.w == 1.0)
	    ambientOcc.w = 1.41421356237;
	}
	}*/

	ambientOccPos = ambientOccArr[gl_VertexID % 4];


	blockLight = 1.0f;// ssboData.lightData[(index * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) + (((coords.x >> 3) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) + ((coords.y >> 3) * (CHUNK_SIZE + 1)) + (coords.z >> 3))];

	// blockLight = float(regionPropertiesBufferData.regionData[gl_BaseInstance].chunkProperties[gl_DrawID].drawCommand.baseInstance);
}