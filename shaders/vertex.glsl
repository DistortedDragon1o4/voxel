R"(
#version 460 core
#extension GL_ARB_gpu_shader_fp64 : enable

layout (location = 0) in int dataBlock1;
layout (location = 1) in int dataBlock2;

layout (binding = 3, std430) buffer SSBOData
{
  float lightData[];
} ssboData;

layout (binding = 4, std430) buffer BlockData
{
  int data[];
} blockData;


out vec2 texCoord;
out float ambientOcc;
out float blockID;
out vec3 normal;
out float camDistance;
out float blockLight;

uniform int index;

uniform dmat4 cameraMatrix;
uniform ivec3 chunkID;
uniform vec3 camPos;

int CHUNK_SIZE = 16;

void main() {

  const int coordMask = 0xff;
  ivec3 coords = ivec3(((dataBlock1 & (coordMask << 16)) >> 16), ((dataBlock1 & (coordMask << 8)) >> 8), (dataBlock1 & coordMask));
  dvec3 mesh = dvec3(double(coords.x / 8.0) + double(chunkID.x * CHUNK_SIZE), double(coords.y / 8.0) + double(chunkID.y * CHUNK_SIZE), double(coords.z / 8.0) + double(chunkID.z * CHUNK_SIZE));
  gl_Position = vec4(cameraMatrix * dvec4(mesh, 1.0));

  camDistance = distance(vec3(mesh), camPos);

  const int texMask = 0xff;
  int texMap = (dataBlock1 & (texMask << 24)) >> 24;
  ivec2 texInt = ivec2(texMap >> 4, texMap & 0xf);
  texCoord = vec2(float(texInt.x / 8.0), float(texInt.y / 8.0));

  blockID = int(dataBlock2 >> 4);

  ambientOcc = float(int(dataBlock2 & 1) / 10.0);

  const int normalMask = 0xe;
  int normalMap = (dataBlock2 & normalMask) >> 1;
  if (normalMap == 5)
    normal = vec3(0.0f, 0.0f, 1.0f);
  else if (normalMap == 4)
    normal = vec3(0.0f, 0.0f, -1.0f);
  else if (normalMap == 3)
    normal = vec3(1.0f, 0.0f, 0.0f);
  else if (normalMap == 2)
    normal = vec3(-1.0f, 0.0f, 0.0f);
  else if (normalMap == 1)
    normal = vec3(0.0f, 1.0f, 0.0f);
  else if (normalMap == 0)
    normal = vec3(0.0f, -1.0f, 0.0f);

  blockLight = ssboData.lightData[(index * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) + (((coords.x >> 3) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)) + ((coords.y >> 3) * (CHUNK_SIZE + 1)) + (coords.z >> 3))];
}
)"
