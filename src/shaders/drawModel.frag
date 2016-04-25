///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 exNormal;
in vec4 exPosition;
in vec2 exTexCoords;

out vec4 outColor;

layout(location = 0) uniform vec3 diffColor;
layout(location = 1) uniform sampler2D diffuseUnit;
layout(location = 2) uniform sampler2D maskUnit;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 0) uniform CameraBuffer {
	Camera cam;
};

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	mat4 MTShadowMatrix;
	vec3 lightDir;
	uint voxelDraw;
	uint view;
	uint voxelRes;
	uint voxelLayer;
	uint numMipLevels;
	uint mipLevel;
};

layout (std140, binding = 1) uniform SceneBuffer {
	SceneParams scene;
};

struct ShadeParams {
	vec3 n;
	vec3 s;
	vec3 r;
	vec3 v;
};

ShadeParams calcShadeParams(vec3 normal, vec3 lightDir, vec4 position, mat4 WTV) {
	ShadeParams result;
	result.n = normalize(normal);
	result.s = normalize(mat3(WTV) * lightDir);
	result.r = normalize(2 * result.n * dot(result.s, result.n) - result.s);
	result.v = normalize(-(position.xyz / position.w));
	return result;
}

float calcDiffShade(vec3 s, vec3 n) {
	return max(0.2, dot(s, n));
}

float calcSpecShade(vec3 r, vec3 v, float specCoeff) {
	return max(0.0, pow(dot(r, v), specCoeff));
}

subroutine vec4 SampleColor();

layout(index = 0) subroutine(SampleColor) 
vec4 DiffuseColor() {
	return vec4(diffColor, 1.0f);
}

layout(index = 1) subroutine(SampleColor)
vec4 TextureColor() {
	return vec4(texture(diffuseUnit, exTexCoords).rgb, 1.0f);
}

layout(index = 2) subroutine(SampleColor)
vec4 MaskColor() {
	return vec4(texture(diffuseUnit, exTexCoords).rgb, texture(maskUnit, exTexCoords).r);
}

layout(location = 0) subroutine uniform SampleColor GetColor;

void main()
{	
	// Calculate all necessary parameters
	ShadeParams sp = calcShadeParams(exNormal, scene.lightDir, exPosition, cam.WTVmatrix);

	// Calculate diffuse and specular light
	float diff = calcDiffShade(sp.s, sp.n);
	float spec = calcSpecShade(sp.r, sp.v, 5.0f);

	// Set constant color for textured models
	vec4 color = GetColor();

	// Apply light to texture
	color.rgb *= (diff + spec);

	// Output complete color
	outColor = color;
}