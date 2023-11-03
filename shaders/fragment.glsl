R"(
#version 460 core

in vec3 normal;
in vec2 texCoord;
in float ambientOcc;
in float blockLight;
in float camDistance;
in float blockID;

uniform sampler2DArray array;
uniform vec3 camDir;
uniform vec3 sunDir;

out vec4 FragColor;

float sunStrength = 4.0f;
vec3 sunColor = vec3(0.9882f, 0.9450f, 0.8117f);
float moonStrength = 1.0f;
vec3 moonColor = vec3(0.6117f, 0.7450f, 0.7882f);

float ambientLight = 2.63f;

float fog(float magnitude, float sharpness, float cutoff) {
  float x = camDistance - cutoff;
  x = sharpness * x;
  float r = (1 + (x * sign(x)) - abs(sign(x))) / (x + sign(x));
  r = -1.0f * magnitude * r;

  if (r > 1.0f)
    return 1.0f;
  else if (r < 0.0f)
    return 0.0f;
  else
    return r;
}

float limitPos(float x) {
  if (x < 0.0f)
    return 0.0f;
  else
    return x;
}

void main() {
  if (texture(array, vec3(texCoord, blockID - 1)).a < 0.1)
    discard;
  vec3 backgroundColor = vec3(0.2f, 0.5f, 0.9f);
  float ambientOcc = 1.0f - (4 * ambientOcc);
  float fogStrength = fog(1.0f, 0.5f, float(16 * 19));

  vec4 frag = texture(array, vec3(texCoord, blockID - 1)) * 0.1f;

  vec3 sunLight;
  vec3 moonLight;
  float specularIntensity = 1.0f;

  if (sunDir.y > -0.1) {
    float sunIntensity = dot(normal, sunDir) * sunStrength;
    sunIntensity = limitPos(sunIntensity);
    sunLight = sunColor * sunIntensity;
    specularIntensity *= limitPos(dot(-reflect(-sunDir, normal), camDir)) / 2.0f;
  } else {
    float sunIntensity = dot(normal, sunDir) * sunStrength;
    sunIntensity = limitPos(sunIntensity);
    float intensity = (1.0f / 0.2f) * limitPos(sunDir.y + 0.3f) * sunIntensity;
    sunLight = sunColor * intensity;
    specularIntensity *= limitPos(dot(-reflect(-sunDir, normal), camDir)) / 2.0f;
  }
  
  if (sunDir.y < 0.1) {
    float moonIntensity = dot(normal, -sunDir) * moonStrength;
    moonIntensity = limitPos(moonIntensity);
    moonLight = moonColor * moonIntensity;
    specularIntensity *= limitPos(dot(-reflect(sunDir, normal), camDir)) / 2.0f;
  } else {
    float moonIntensity = dot(normal, -sunDir) * moonStrength;
    moonIntensity = limitPos(moonIntensity);
    float intensity = (1.0f / 0.2f) * limitPos(-sunDir.y + 0.3f) * moonIntensity;
    moonLight = moonColor * intensity;
    specularIntensity *= limitPos(dot(-reflect(-sunDir, normal), camDir)) / 2.0f;
  }

  
  // float floatBlockLight = float(blockLight);
  vec3 light = (sunLight + moonLight + vec3(ambientLight) + specularIntensity) * ambientOcc * blockLight;

  frag = vec4(frag.xyz * /*float(blockLight)*/light, 1.0f);

  //Applying fog
  vec3 diff = backgroundColor - frag.xyz;
  vec4 color = vec4(frag.xyz + ((1.0f - fogStrength) * diff), 1.0f);

  FragColor = color/*vec4((1.0f - fogStrength) * backgroundColor, 1.0f)*/;
}
)"
