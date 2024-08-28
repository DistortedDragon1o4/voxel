#version 460 core

struct MemRegUnit {
	int x;
	int y;
	int z;
	int memoryIndex;
	int size;							// In bytes (size of the mesh)
	int lightMemoryIndex;
	int lightSize;						// In bytes (size of the mesh)
};

layout (binding = 2, std430) buffer MemoryRegister {
 	MemRegUnit unit[];
} memoryRegister;

layout (binding = 3, std430) buffer MemoryBlock {
	int v[];
} memoryBlock;


out int ERROR;

out vec2 texCoord;

out float blockID;
out vec3 normal;
out float camDistance;

out flat int index;
out vec3 crntCoord;

out vec4 ambientOcc;
out vec2 ambientOccPos;

out flat int lodLevel;


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

uint indices[6] = {
	0, 1, 2,
	2, 3, 0
};

void main() {

	int dataBlock1 = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[gl_VertexID % 6]))];
	int dataBlock2 = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[gl_VertexID % 6])) + 1];

	ivec3 chunkID = ivec3(memoryRegister.unit[gl_BaseInstance].x, memoryRegister.unit[gl_BaseInstance].y, memoryRegister.unit[gl_BaseInstance].z);
	index = gl_BaseInstance;

	// Stuff left to do to fix precision issues

	lodLevel = int((dataBlock2 >> 14) & 0xf);

	const int coordMask = 0x3ff;
	ivec3 coords = ivec3(((dataBlock1 >> 20) & coordMask), ((dataBlock1 >> 10) & coordMask), (dataBlock1 & coordMask));
	coords *= (1 << lodLevel);
	vec3 mesh = vec3(float(coords.x / 16.0) + (chunkID.x * CHUNK_SIZE), float(coords.y / 16.0) + (chunkID.y * CHUNK_SIZE), float(coords.z / 16.0) + (chunkID.z * CHUNK_SIZE));
	mesh = mesh - camPos;
	gl_Position = vec4(cameraMatrix * vec4(mesh, 1.0));

	int length = abs((memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6))))] & coordMask) - (memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 2))] & coordMask));
	int multiplier = length / 16;

	camDistance = distance(vec3(mesh), camPos);

	const int texMask = 0x3ff;
	int texMap = (dataBlock2 >> 4) & texMask;
	ivec2 texInt = ivec2(texMap >> 5, texMap & 0x1f);
	texCoord = float(1 << lodLevel) * vec2(float(texInt.x / 16.0), float(texInt.y / 16.0));

	blockID = int(dataBlock2 >> 18);

	const int normalMask = 0x7;
	int normalIdx = (dataBlock2 >> 1) & normalMask;
	normal = normalArr[normalIdx];

	if (length != 0) {
		if (normalIdx == 2 || normalIdx == 3) {
			texCoord.x *= multiplier;
		}
		if (normalIdx == 0 || normalIdx == 1) {
			texCoord.y *= multiplier;
		}
	}

	vec4 ambientOccLocal;
	ambientOccLocal.x = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 0)) + 1] & 1)];
	ambientOccLocal.y = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 1)) + 1] & 1)];
	ambientOccLocal.z = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 2)) + 1] & 1)];
	ambientOccLocal.w = ambientOccMap[int(memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + 3)) + 1] & 1)];

	ambientOcc = ambientOccLocal;

	ambientOccPos = ambientOccArr[indices[gl_VertexID % 6] % 4];

	crntCoord = float(1 << lodLevel) * vec3(coords) / 16.0;

	ERROR = memoryBlock.v[memoryRegister.unit[gl_BaseInstance].memoryIndex / 4];
}