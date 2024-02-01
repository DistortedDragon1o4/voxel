#version 460 core


in vec3 normal;
in vec2 texCoord;

// in float ambientOcc;

in float camDistance;
in float blockID;

in vec4 ambientOcc;
in vec2 ambientOccPos;

in vec3 vertexLightValues[4];

uniform sampler2DArray array;
uniform vec3 camDir;
uniform vec3 sunDir;

out vec4 FragColor;

float sunStrength = 1.0f;
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

float getAmbientOcc() {
	float x1 = mix(ambientOcc.x, ambientOcc.y, smoothstep(0, 1, ambientOccPos.x));
	float x2 = mix(ambientOcc.w, ambientOcc.z, smoothstep(0, 1, ambientOccPos.x));
	float res = mix(x1, x2, smoothstep(0, 1, ambientOccPos.y));

	float k = 1.0 - res;

	return sqrt(1.0 - pow(k - 1.265, 8.0));
}

vec3 getLighting(vec2 position) {
	vec3 x1 = mix(vertexLightValues[0], vertexLightValues[1], position.x);
	vec3 x2 = mix(vertexLightValues[3], vertexLightValues[2], position.x);
	return mix(x1, x2, position.y);
}

float gamma = 2.2;

void main() {

	// Sampling the texture
	vec4 frag = texture(array, vec3(texCoord, blockID - 1));

	// Discarding transparent stuff
	if (frag.a < 0.1)
		discard;

	// Gamma
	frag = vec4(pow(frag.rgb, vec3(gamma)), 1.0);

	vec3 backgroundColor = vec3(0.2f, 0.5f, 0.9f);

	
	float ambientOccLocal = getAmbientOcc();

	vec3 blockLight = getLighting(ambientOccPos);



	float fogStrength = fog(1.0f, 0.5f, float(16 * 19));



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


	vec3 light = (vec3(ambientLight) + blockLight + sunLight) * ambientOccLocal;

	frag = vec4(frag.xyz * light, 1.0f);

	frag = vec4(1.0) + (vec4(1.0) / vec4(-1.0) - frag);
	frag = vec4(frag.xyz, 1.0);

	//Applying fog
	vec3 diff = backgroundColor - frag.xyz;
	vec4 color = vec4(pow(frag.rgb, vec3(1.0 / gamma))/* + ((1.0f - fogStrength) * diff)*/, 1.0f);

	FragColor = color/*vec4((1.0f - fogStrength) * backgroundColor, 1.0f)*/;
}
