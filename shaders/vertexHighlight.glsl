R"(
#version 460 core
#extension GL_ARB_gpu_shader_fp64 : enable

layout (location = 0) in vec3 aData;

uniform dmat4 cameraMatrix;

uniform vec3 Pos;

void main() {
  dvec4 mesh = dvec4(dvec3(Pos + aData), 1.0f);

  gl_Position = vec4(cameraMatrix * mesh);
}
)"
