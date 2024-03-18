#version 460 core

struct MemRegUnit {
	int x;
	int y;
	int z;
	int memoryIndex;
	int size;							// In bytes (size of the mesh)
};

struct ChunkLightData {
	int data[34 * 34 * 34];
};

layout (binding = 2, std430) buffer MemoryRegister {
 	MemRegUnit unit[];
} memoryRegister;

layout (binding = 3, std430) buffer MemoryBlock {
	int v[];
} memoryBlock;

layout (binding = 4, std430) buffer LightDataBuffer {
 	ChunkLightData lightData[];
} lightDataBuffer;


out int ERROR;

out vec2 texCoord;

out float blockID;
out vec3 normal;
out float camDistance;

out vec3 vertexLightValues[4];

out vec4 ambientOcc;

out vec2 ambientOccPos;


uniform mat4 cameraMatrix;
uniform vec3 camPos;

// DO NOT CHANGE
float CHUNK_SIZE = 32.0;

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

int surroundingLightsLUT[6 * 4] = {
	2, 3, 6, 7,
	0, 1, 4, 5,
	4, 5, 6, 7,
	0, 1, 2, 3,
	1, 3, 5, 7,
	0, 2, 4, 6
};

ivec3 faceVertexCoords[6 * 4] = {
	ivec3(1, 0, 1), ivec3(0, 0, 1), ivec3(0, 0, 0), ivec3(1, 0, 0),
	ivec3(1, 1, 0), ivec3(0, 1, 0), ivec3(0, 1, 1), ivec3(1, 1, 1),
	ivec3(0, 1, 1), ivec3(0, 1, 0), ivec3(0, 0, 0), ivec3(0, 0, 1),
	ivec3(1, 1, 0), ivec3(1, 1, 1), ivec3(1, 0, 1), ivec3(1, 0, 0),
	ivec3(0, 1, 0), ivec3(1, 1, 0), ivec3(1, 0, 0), ivec3(0, 0, 0),
	ivec3(1, 1, 1), ivec3(0, 1, 1), ivec3(0, 0, 1), ivec3(1, 0, 1),
};

// these are in the opposite order sadly
bool shallReduceByOneX[6 * 4] = {
	true, false, false, true,
	false, true, true, false,
	true, true, true, true,
	false, false, false, false,
	true, false, false, true,
	true, false, false, true
};

bool shallReduceByOneY[6 * 4] = {
	true, true, false, false,
	true, true, false, false,
	true, true, false, false,
	true, true, false, false,
	true, true, true, true,
	false, false, false, false
};

bool shallReduceByOneZ[6 * 4] = {
	true, true, true, true,
	false, false, false, false,
	false, true, true, false,
	true, false, false, true,
	false, false, true, true,
	true, true, false, false 
};

uint coordsToLightIndex(ivec3 coords) {
	return ((coords.x + 1) * 34 * 34) + ((coords.y + 1) * 34) + (coords.z + 1);
}

vec3 vertexLight(ivec3 position, int normalIdx, int index) {
	ivec3 baseLightCoords = position;

	uint surroundingBlockLightIndices[8];
	surroundingBlockLightIndices[0] = coordsToLightIndex(ivec3(baseLightCoords.x, baseLightCoords.y, baseLightCoords.z));
	surroundingBlockLightIndices[1] = coordsToLightIndex(ivec3(baseLightCoords.x, baseLightCoords.y, baseLightCoords.z - 1));
	surroundingBlockLightIndices[2] = coordsToLightIndex(ivec3(baseLightCoords.x, baseLightCoords.y - 1, baseLightCoords.z));
	surroundingBlockLightIndices[3] = coordsToLightIndex(ivec3(baseLightCoords.x, baseLightCoords.y - 1, baseLightCoords.z - 1));
	surroundingBlockLightIndices[4] = coordsToLightIndex(ivec3(baseLightCoords.x - 1, baseLightCoords.y, baseLightCoords.z));
	surroundingBlockLightIndices[5] = coordsToLightIndex(ivec3(baseLightCoords.x - 1, baseLightCoords.y, baseLightCoords.z - 1));
	surroundingBlockLightIndices[6] = coordsToLightIndex(ivec3(baseLightCoords.x - 1, baseLightCoords.y - 1, baseLightCoords.z));
	surroundingBlockLightIndices[7] = coordsToLightIndex(ivec3(baseLightCoords.x - 1, baseLightCoords.y - 1, baseLightCoords.z - 1));

	int importantSurroundingLights[4];
	importantSurroundingLights[0] = lightDataBuffer.lightData[index].data[surroundingBlockLightIndices[surroundingLightsLUT[(normalIdx * 4) + 0]]];
	importantSurroundingLights[1] = lightDataBuffer.lightData[index].data[surroundingBlockLightIndices[surroundingLightsLUT[(normalIdx * 4) + 1]]];
	importantSurroundingLights[2] = lightDataBuffer.lightData[index].data[surroundingBlockLightIndices[surroundingLightsLUT[(normalIdx * 4) + 2]]];
	importantSurroundingLights[3] = lightDataBuffer.lightData[index].data[surroundingBlockLightIndices[surroundingLightsLUT[(normalIdx * 4) + 3]]];

	vec3 importantSurroundingLightColours[4];
	importantSurroundingLightColours[0] = vec3(float((importantSurroundingLights[0] >> 16) & 0xff), float((importantSurroundingLights[0] >> 8) & 0xff), float(importantSurroundingLights[0] & 0xff));
	importantSurroundingLightColours[1] = vec3(float((importantSurroundingLights[1] >> 16) & 0xff), float((importantSurroundingLights[1] >> 8) & 0xff), float(importantSurroundingLights[1] & 0xff));
	importantSurroundingLightColours[2] = vec3(float((importantSurroundingLights[2] >> 16) & 0xff), float((importantSurroundingLights[2] >> 8) & 0xff), float(importantSurroundingLights[2] & 0xff));
	importantSurroundingLightColours[3] = vec3(float((importantSurroundingLights[3] >> 16) & 0xff), float((importantSurroundingLights[3] >> 8) & 0xff), float(importantSurroundingLights[3] & 0xff));

	return (importantSurroundingLightColours[0] + importantSurroundingLightColours[1] + importantSurroundingLightColours[2] + importantSurroundingLightColours[3]) / (4.0 * 255.0);
}

uint indices[6] = {
	0, 1, 2,
	2, 3, 0
};

void main() {

	int dataBlock1 = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[gl_VertexID % 6]))];
	int dataBlock2 = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[gl_VertexID % 6])) + 1];

	ivec3 chunkID = ivec3(memoryRegister.unit[gl_BaseInstance].x, memoryRegister.unit[gl_BaseInstance].y, memoryRegister.unit[gl_BaseInstance].z);
	int index = gl_BaseInstance;

	// Stuff left to do to fix precision issues

	const int coordMask = 0x3ff;
	ivec3 coords = ivec3(((dataBlock1 >> 20) & coordMask), ((dataBlock1 >> 10) & coordMask), (dataBlock1 & coordMask));
	vec3 mesh = vec3(float(coords.x / 16.0) + (chunkID.x * CHUNK_SIZE), float(coords.y / 16.0) + (chunkID.y * CHUNK_SIZE), float(coords.z / 16.0) + (chunkID.z * CHUNK_SIZE));
	mesh = mesh - camPos;
	gl_Position = vec4(cameraMatrix * vec4(mesh, 1.0));


	camDistance = distance(vec3(mesh), camPos);

	const int texMask = 0x3ff;
	int texMap = (dataBlock2 >> 4) & texMask;
	ivec2 texInt = ivec2(texMap >> 5, texMap & 0x1f);
	texCoord = vec2(float(texInt.x / 16.0), float(texInt.y / 16.0));

	blockID = int(dataBlock2 >> 14);

	const int normalMask = 0x7;
	int normalIdx = (dataBlock2 >> 1) & normalMask;
	normal = normalArr[normalIdx];

	vec4 ambientOccLocal;
	ambientOccLocal.x = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 0)) + 1] & 1)];
	ambientOccLocal.y = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 1)) + 1] & 1)];
	ambientOccLocal.z = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 2)) + 1] & 1)];
	ambientOccLocal.w = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 3)) + 1] & 1)];

	ambientOcc = ambientOccLocal;

	ambientOccPos = ambientOccArr[indices[gl_VertexID % 6] % 4];


	vec3 vertexCoords = vec3(coords) / 16.0;

	ivec3 crntBlockCoords;

	crntBlockCoords.x = ((shallReduceByOneX[((5 - normalIdx) * 4) + (indices[gl_VertexID % 6] % 4)]) && floor(vertexCoords.x) == vertexCoords.x) ? int(floor(vertexCoords.x) - 1.0) : int(floor(vertexCoords.x));
	crntBlockCoords.y = ((shallReduceByOneY[((5 - normalIdx) * 4) + (indices[gl_VertexID % 6] % 4)]) && floor(vertexCoords.y) == vertexCoords.y) ? int(floor(vertexCoords.y) - 1.0) : int(floor(vertexCoords.y));
	crntBlockCoords.z = ((shallReduceByOneZ[((5 - normalIdx) * 4) + (indices[gl_VertexID % 6] % 4)]) && floor(vertexCoords.z) == vertexCoords.z) ? int(floor(vertexCoords.z) - 1.0) : int(floor(vertexCoords.z));

	vertexLightValues[0] = vertexLight(faceVertexCoords[(4 * normalIdx) + 0] + crntBlockCoords, normalIdx, index);
	vertexLightValues[1] = vertexLight(faceVertexCoords[(4 * normalIdx) + 1] + crntBlockCoords, normalIdx, index);
	vertexLightValues[2] = vertexLight(faceVertexCoords[(4 * normalIdx) + 2] + crntBlockCoords, normalIdx, index);
	vertexLightValues[3] = vertexLight(faceVertexCoords[(4 * normalIdx) + 3] + crntBlockCoords, normalIdx, index);

	ERROR = memoryBlock.v[memoryRegister.unit[gl_BaseInstance].memoryIndex / 4];
}