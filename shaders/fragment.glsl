#version 460 core

struct ChunkLightData {
	int data[34 * 34 * 34];
};

layout (binding = 4, std430) buffer LightDataBuffer {
 	ChunkLightData lightData[];
} lightDataBuffer;

in flat int ERROR;

in vec2 texCoord;

in float blockID;
in vec3 normal;
in float camDistance;

in flat int index;
in vec3 crntCoord;

in vec4 ambientOcc;
in vec2 ambientOccPos;

in vec3 worldSpaceCoords;

out vec4 FragColor;



uniform sampler2DArray array;
uniform sampler2DArray shadowMap;

uniform vec3 camPos;
uniform vec3 camDir;
uniform vec3 sunDir;

uniform mat4 lightSpaceMatrix0;
uniform mat4 lightSpaceMatrix1;
uniform mat4 lightSpaceMatrix2;
uniform mat4 lightSpaceMatrix3;


// DO NOT CHANGE
int CHUNK_SIZE = 32;

float SHADOW_DISTANCE = 25.0 * 2.0;

float sunStrength = 0.0f;
vec3 sunColor = vec3(0.9882f, 0.9450f, 0.8117f);
float moonStrength = 1.0f;
vec3 moonColor = vec3(0.6117f, 0.7450f, 0.7882f);

float ambientLight = 0.0113f;

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

vec3 spherical2cartesian(float r, float theta, float phi) {
	return r * vec3(cos(theta) * cos(phi), sin(theta), cos(theta) * sin(phi));
}

float rand(vec3 co){
    return (fract(sin(dot(co, vec3(12.9898, 78.233, 45.857))) * 43758.5453) - 0.5) * 2.0;
}

bool checkDepth(vec3 coords, mat4 matrix, float layer) {
	vec4 lightSpaceCoords = matrix * vec4(coords, 1.0);

	coords = lightSpaceCoords.xyz / lightSpaceCoords.w;
	vec3 texCoords = (coords / 2.0) + 0.5;

	float closestDepth = texture(shadowMap, vec3(texCoords.xy, layer)).r;
	float crntDepth = texCoords.z;

	return (crntDepth >= closestDepth + 0.00001) && (abs(coords.x) <= 1.0) && (abs(coords.y) <= 1.0) && (abs(coords.z) <= 1.0);
}

float sunLightIntensity(vec3 worldSpaceCoord) {
	worldSpaceCoord = floor(worldSpaceCoord * 32) / 32;
	worldSpaceCoord = worldSpaceCoord + (0.03125 * normal);

	float spec = pow(clamp(dot(sunDir, normalize(reflect(worldSpaceCoords - camPos, normal))), 0.0, 1.0), 16);

	return (checkDepth(worldSpaceCoord, lightSpaceMatrix0, 0.0) || checkDepth(worldSpaceCoord, lightSpaceMatrix1, 1.0) || checkDepth(worldSpaceCoord, lightSpaceMatrix2, 2.0) || checkDepth(worldSpaceCoord, lightSpaceMatrix3, 3.0)) ? 0.33 : (1.89 + spec);
}

float getAmbientOcc() {
	float x1 = mix(ambientOcc.x, ambientOcc.y, smoothstep(0, 1, ambientOccPos.x));
	float x2 = mix(ambientOcc.w, ambientOcc.z, smoothstep(0, 1, ambientOccPos.x));
	float res = mix(x1, x2, smoothstep(0, 1, ambientOccPos.y));

	float k = 1.0 - res;

	return sqrt(1.0 - pow(k - 1.265, 8.0));
}

vec3 getLighting(vec3 position) {
	vec3 surroundingLight[8];
	ivec3 crntPosition = ivec3(floor(position - vec3(0.5)));

	int x;
	int y;
	int z;

	for (int i = 0; i < 8; i++) {
		x = i >> 2;
		y = (i >> 1) & 1;
		z = i & 1;
		int lightVal = lightDataBuffer.lightData[index].data[((crntPosition.x + x + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)) + ((crntPosition.y + y + 1) * (CHUNK_SIZE + 2)) + (crntPosition.z + z + 1)];
		surroundingLight[i].x = (lightVal >> 16) & 0xff;
		surroundingLight[i].y = (lightVal >> 8) & 0xff;
		surroundingLight[i].z = lightVal & 0xff;
	}

	vec3 relativePosition = position - vec3(crntPosition) - vec3(0.5);

	vec3 z1 = mix(surroundingLight[0], surroundingLight[1], relativePosition.z);
	vec3 z2 = mix(surroundingLight[2], surroundingLight[3], relativePosition.z);

	vec3 y1 = mix(z1, z2, relativePosition.y);

	z1 = mix(surroundingLight[4], surroundingLight[5], relativePosition.z);
	z2 = mix(surroundingLight[6], surroundingLight[7], relativePosition.z);

	vec3 y2 = mix(z1, z2, relativePosition.y);

	return 1.78 * (mix(y1, y2, relativePosition.x) / 255.0);
}

float gamma = 2.2;

void main() {

	// Sampling the texture
	vec4 frag = texture(array, vec3(texCoord, blockID - 1));
	// vec4 frag = vec4(vec3(texture(shadowMap, texCoord).r), 1.0);

	// Discarding transparent stuff
	if (frag.a < 0.1)
		discard;

	// Gamma
	frag = vec4(pow(frag.rgb, vec3(gamma)), 1.0);

	vec3 backgroundColor = vec3(0.2f, 0.5f, 0.9f);

	
	float ambientOccLocal = getAmbientOcc();

	vec3 newCrntCoord = floor(crntCoord * 16.0) / 16.0;
	float factor = 0.117;

	vec3 blockLight = getLighting(newCrntCoord + (factor * vec3(rand(1.366 * newCrntCoord), rand(2.346 * newCrntCoord), rand(3.241 * newCrntCoord))));



	float fogStrength = fog(1.0f, 0.5f, float(16 * 19));



	vec3 sunLight;
	vec3 moonLight;
	float specularIntensity = 1.0f;

	// if (sunDir.y > -0.1) {
	// 	float sunIntensity = dot(normal, sunDir) * sunStrength;
	// 	sunIntensity = limitPos(sunIntensity);
	// 	sunLight = sunColor * sunIntensity;
	// 	specularIntensity *= limitPos(dot(-reflect(-sunDir, normal), camDir)) / 2.0f;
	// } else {
	// 	float sunIntensity = dot(normal, sunDir) * sunStrength;
	// 	sunIntensity = limitPos(sunIntensity);
	// 	float intensity = (1.0f / 0.2f) * limitPos(sunDir.y + 0.3f) * sunIntensity;
	// 	sunLight = sunColor * intensity;
	// 	specularIntensity *= limitPos(dot(-reflect(-sunDir, normal), camDir)) / 2.0f;
	// }

	// if (sunDir.y < 0.1) {
	// 	float moonIntensity = dot(normal, -sunDir) * moonStrength;
	// 	moonIntensity = limitPos(moonIntensity);
	// 	moonLight = moonColor * moonIntensity;
	// 	specularIntensity *= limitPos(dot(-reflect(sunDir, normal), camDir)) / 2.0f;
	// } else {
	// 	float moonIntensity = dot(normal, -sunDir) * moonStrength;
	// 	moonIntensity = limitPos(moonIntensity);
	// 	float intensity = (1.0f / 0.2f) * limitPos(-sunDir.y + 0.3f) * moonIntensity;
	// 	moonLight = moonColor * intensity;
	// 	specularIntensity *= limitPos(dot(-reflect(-sunDir, normal), camDir)) / 2.0f;
	// }

	// if (distance(worldSpaceCoords, camPos) <= SHADOW_DISTANCE)
		sunLight = sunColor * sunLightIntensity(worldSpaceCoords) * clamp(dot(sunDir, normal), 0.33, 1.0);
	// else
	// 	sunLight = sunColor * 1.89 * clamp(dot(sunDir, normal), 0.33, 1.0);

	


	vec3 light = (vec3(ambientLight) + blockLight + sunLight);

	frag = vec4(frag.xyz * light, 1.0f);

	frag = vec4(1.0) + (vec4(1.0) / vec4(-1.0) - frag);
	frag = vec4(frag.xyz, 1.0);

	//Applying fog
	vec3 diff = backgroundColor - frag.xyz;
	vec4 color = vec4(pow(frag.rgb, vec3(1.0 / gamma))/* + ((1.0f - fogStrength) * diff)*/, 1.0f);

	FragColor = color/*vec4((1.0f - fogStrength) * backgroundColor, 1.0f)*/;
}
