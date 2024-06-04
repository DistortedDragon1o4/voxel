#version 460 core

#extension GL_ARB_shader_viewport_layer_array : enable
#extension GL_AMD_vertex_shader_layer : enable
#extension GL_NV_viewport_array2 : enable

struct MemRegUnit {
	int x;
	int y;
	int z;
	int memoryIndex;
	int size;							// In bytes (size of the mesh)
};

layout (binding = 2, std430) buffer MemoryRegister {
 	MemRegUnit unit[];
} memoryRegister;

layout (binding = 3, std430) buffer MemoryBlock {
	int v[];
} memoryBlock;

out int ERROR;

uniform mat4 lightSpaceMatrix0;
uniform mat4 lightSpaceMatrix1;
uniform mat4 lightSpaceMatrix2;
uniform mat4 lightSpaceMatrix3;

// DO NOT CHANGE
float CHUNK_SIZE = 32.0;

uint indices[6] = {
	0, 1, 2,
	2, 3, 0
};

const int coordMask = 0x3ff;
ivec3 chunkID = ivec3(memoryRegister.unit[gl_BaseInstance].x, memoryRegister.unit[gl_BaseInstance].y, memoryRegister.unit[gl_BaseInstance].z);

bool checkIfQuadInFrustum(mat4 matrix) {
	int dataBlock[4];
	dataBlock[0] = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[0]))];
	dataBlock[1] = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[1]))];
	dataBlock[2] = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[2]))];
	dataBlock[3] = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[3]))];

	ivec3 coords[4];
	vec3 meshes[4];
	vec3 meshInLightSpace[4];

	coords[0] = ivec3(((dataBlock[0] >> 20) & coordMask), ((dataBlock[0] >> 10) & coordMask), (dataBlock[0] & coordMask));
	meshes[0] = vec3(float(coords[0].x / 16.0) + (chunkID.x * CHUNK_SIZE), float(coords[0].y / 16.0) + (chunkID.y * CHUNK_SIZE), float(coords[0].z / 16.0) + (chunkID.z * CHUNK_SIZE));
	meshInLightSpace[0] = abs((matrix * vec4(meshes[0], 1.0)).xyz);

	coords[1] = ivec3(((dataBlock[1] >> 20) & coordMask), ((dataBlock[1] >> 10) & coordMask), (dataBlock[1] & coordMask));
	meshes[1] = vec3(float(coords[1].x / 16.0) + (chunkID.x * CHUNK_SIZE), float(coords[1].y / 16.0) + (chunkID.y * CHUNK_SIZE), float(coords[1].z / 16.0) + (chunkID.z * CHUNK_SIZE));
	meshInLightSpace[1] = abs((matrix * vec4(meshes[1], 1.0)).xyz);

	coords[2] = ivec3(((dataBlock[2] >> 20) & coordMask), ((dataBlock[2] >> 10) & coordMask), (dataBlock[2] & coordMask));
	meshes[2] = vec3(float(coords[2].x / 16.0) + (chunkID.x * CHUNK_SIZE), float(coords[2].y / 16.0) + (chunkID.y * CHUNK_SIZE), float(coords[2].z / 16.0) + (chunkID.z * CHUNK_SIZE));
	meshInLightSpace[2] = abs((matrix * vec4(meshes[2], 1.0)).xyz);

	coords[3] = ivec3(((dataBlock[3] >> 20) & coordMask), ((dataBlock[3] >> 10) & coordMask), (dataBlock[3] & coordMask));
	meshes[3] = vec3(float(coords[3].x / 16.0) + (chunkID.x * CHUNK_SIZE), float(coords[3].y / 16.0) + (chunkID.y * CHUNK_SIZE), float(coords[3].z / 16.0) + (chunkID.z * CHUNK_SIZE));
	meshInLightSpace[3] = abs((matrix * vec4(meshes[3], 1.0)).xyz);

	return (meshInLightSpace[0].x < 1.0 && meshInLightSpace[0].y < 1.0) && (meshInLightSpace[1].x < 1.0 && meshInLightSpace[1].y < 1.0) && (meshInLightSpace[2].x < 1.0 && meshInLightSpace[2].y < 1.0) && (meshInLightSpace[3].x < 1.0 && meshInLightSpace[3].y < 1.0);
}

void main() {

	int dataBlock1 = memoryBlock.v[((memoryRegister.unit[gl_BaseInstance].memoryIndex / 4) + 1) + (2 * ((4 * (gl_VertexID / 6)) + indices[gl_VertexID % 6]))];

	// Stuff left to do to fix precision issues
	
	ivec3 coords = ivec3(((dataBlock1 >> 20) & coordMask), ((dataBlock1 >> 10) & coordMask), (dataBlock1 & coordMask));
	vec3 mesh = vec3(float(coords.x / 16.0) + (chunkID.x * CHUNK_SIZE), float(coords.y / 16.0) + (chunkID.y * CHUNK_SIZE), float(coords.z / 16.0) + (chunkID.z * CHUNK_SIZE));
	
	vec3 meshInLightSpace0 = (lightSpaceMatrix0 * vec4(mesh, 1.0)).xyz;
	vec3 meshInLightSpace1 = (lightSpaceMatrix1 * vec4(mesh, 1.0)).xyz;
	vec3 meshInLightSpace2 = (lightSpaceMatrix2 * vec4(mesh, 1.0)).xyz;
	vec3 meshInLightSpace3 = (lightSpaceMatrix3 * vec4(mesh, 1.0)).xyz;

	if (checkIfQuadInFrustum(lightSpaceMatrix0)) {
		gl_Layer = 0;
		gl_Position = vec4(meshInLightSpace0, 1.0);
	} else if (checkIfQuadInFrustum(lightSpaceMatrix1)) {
		gl_Layer = 1;
		gl_Position = vec4(meshInLightSpace1, 1.0);
	} else if (checkIfQuadInFrustum(lightSpaceMatrix2)) {
		gl_Layer = 2;
		gl_Position = vec4(meshInLightSpace2, 1.0);
	} else {
		gl_Layer = 3;
		gl_Position = vec4(meshInLightSpace3, 1.0);
	}



}