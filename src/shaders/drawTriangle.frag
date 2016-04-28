///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430
#define M_PI 3.1415926535897932384626433832795

in vec2 exTexCoords;
out vec4 outColor;

layout(location = 3) uniform usampler2DArray voxelTextures;
layout(location = 4) uniform usampler3D voxelData;
layout(location = 6) uniform sampler2D shadowMap;
layout(location = 8) uniform sampler2DArray sceneTex;
layout(location = 9) uniform sampler2D sceneDepth;

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

struct VoxelData {
	vec4 color;
	uint light;
	uint count;
};

VoxelData unpackARGB8(uint bytesIn) {
	VoxelData data;
	uvec3 uiColor;

	// Put a first to improve max operation but it should not be very noticable
	data.light = (bytesIn & 0xF0000000) >> 28;
	data.count = (bytesIn & 0x0F000000) >> 24;
	uiColor.r =  (bytesIn & 0x00FF0000) >> 16;
	uiColor.g =  (bytesIn & 0x0000FF00) >> 8;
	uiColor.b =  (bytesIn & 0x000000FF);

	data.color.rgb = vec3(uiColor) / float(data.count) / 31.0f;
	data.color.a = float(sign(data.count));

	return data;
}

vec4 voxelSample(vec3 position, float level) {
	float scale = float(scene.voxelRes >> int(level));
	position *= scale;
	position = position - vec3(0.5f);
	vec3 positionWhole = floor(position);
	vec3 intpol = position - positionWhole;
	vec3 offsets[] = {  vec3(0.0f, 0.0f, 0.0f),
						vec3(0.0f, 0.0f, 1.0f),
						vec3(0.0f, 1.0f, 0.0f),
						vec3(0.0f, 1.0f, 1.0f),
						vec3(1.0f, 0.0f, 0.0f),
						vec3(1.0f, 0.0f, 1.0f),
						vec3(1.0f, 1.0f, 0.0f),
						vec3(1.0f, 1.0f, 1.0f) };
	
	vec4 total = vec4(0.0f);
	for(int i = 0; i < 8; i++) {
		vec3 voxelPos = (positionWhole + offsets[i]) / scale;
		VoxelData voxel = unpackARGB8(textureLod(voxelData, voxelPos, level).r);
		vec3 temp = (1.0f - offsets[i]) * (1.0f - intpol) + offsets[i] * intpol; 
		total += voxel.color * temp.x * temp.y * temp.z;
	}

	return total;
}

subroutine vec4 DrawTexture();

layout(index = 0) subroutine(DrawTexture) 
vec4 ProjectionTexture() {
	return unpackARGB8(texture(voxelTextures, vec3(exTexCoords, float(scene.view))).r).color;
}

layout(index = 1) subroutine(DrawTexture)
vec4 VoxelTexure() {
	float size = float(scene.voxelRes >> scene.mipLevel);
	float depth = float(scene.voxelLayer) / size;
	return unpackARGB8(textureLod(voxelData, vec3(exTexCoords, depth), scene.mipLevel).r).color;
}

layout(index = 2) subroutine(DrawTexture)
vec4 ShadowTexture() {
	return texture(shadowMap, exTexCoords);
}

layout(index = 3) subroutine(DrawTexture)
vec4 SceneColor() {
	return texture(sceneTex, vec3(exTexCoords, 0.0f));
}

layout(index = 4) subroutine(DrawTexture)
vec4 ScenePosition() {
	return texture(sceneTex, vec3(exTexCoords, 1.0f));
}

layout(index = 5) subroutine(DrawTexture)
vec4 SceneNormal() {
	vec4 normal = texture(sceneTex, vec3(exTexCoords, 2.0f));
	//normal.xyz /= 2;
	//normal.xyz += vec3(0.5f);
	return normal;
}

layout(index = 6) subroutine(DrawTexture)
vec4 SceneDepth() {
	return texture(sceneDepth, exTexCoords);
}

float coneTrace60(vec3 startPos, vec3 dir, float maxDist, float voxelSize) {
	
	float accum = 0.0f;
	vec3 samplePos;
	float sampleValue;
	float sampleWeight;
	float sampleLOD = 0.0f;

	for(float dist = voxelSize; dist < maxDist && accum < 1.0f;) {
		samplePos = startPos + dir * dist;
		sampleValue = voxelSample(samplePos, sampleLOD).a;
		sampleWeight = 1.0f - accum;
		accum += sampleValue * sampleWeight;
		sampleLOD += 1.0f;
		dist *= 2.0f;
	}
	return 1.0f - accum;
}


layout(index = 7) subroutine(DrawTexture)
vec4 AmbientOcclusion() {
	vec3 dir[] = {  vec3( 0.000000f, 1.000000f,  0.000000f), 
					vec3( 0.000000f, 0.500000f,  0.866025f), 
					vec3( 0.823639f, 0.500000f,  0.267617f), 
					vec3( 0.509037f, 0.500000f, -0.700629f), 
					vec3(-0.509037f, 0.500000f, -0.700629f), 
					vec3(-0.823639f, 0.500000f,  0.267617f) };
	float sideWeight = 3.0f / 20.0f;
	float weight[] = {  1.0f / 4.0f,
						sideWeight,
						sideWeight,
						sideWeight,
						sideWeight,
						sideWeight };
	
	float voxelSize = sqrt(3.0f) / float(scene.voxelRes);
	float maxDistance = 0.015f;
	vec3 pos = ScenePosition().xyz;
	vec3 norm = normalize(SceneNormal().xyz);
	vec3 tang = normalize(vec3(norm.zz, -norm.x-norm.y));
	vec3 bitang = normalize(cross(norm, tang));
	pos += norm * voxelSize * 1.5f;

	float total = 0.0f;
	for(int i = 0; i < 6; i++) {
		vec3 direction = dir[i].x * tang + dir[i].y * norm + dir[i].z * bitang;
		total += weight[i] * coneTrace60(pos, direction, maxDistance, voxelSize);
	}

	return vec4(vec3(total), 1.0f);
}

layout(location = 0) subroutine uniform DrawTexture SampleTexture;

void main()
{	
	outColor = SampleTexture();
}
