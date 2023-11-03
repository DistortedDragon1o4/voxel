R"(
#version 460 core

out vec4 FragColor;

in vec2 pos;
in vec3 color;
in float texture;

uniform sampler2DArray arraay;

void main() {
  vec4 frag;
  frag = vec4(color, 1.0f);
  FragColor = frag;
}
)"
