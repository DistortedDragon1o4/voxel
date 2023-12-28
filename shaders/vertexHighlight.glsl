#version 460 core

layout (location = 0) in vec3 aData;

uniform mat4 cameraMatrix;

uniform vec3 camPos;

uniform vec3 Pos;

void main() {
  vec4 mesh = vec4(Pos - camPos + aData, 1.0f);

  gl_Position = cameraMatrix * mesh;
}
