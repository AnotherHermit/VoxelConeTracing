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


	float div[] = { 0.0f, 1.0f / float(data.count) };
	int index = int(data.count > 0);

	data.color.rgb = vec3(uiColor) / 31.0f * div[index];
	data.color.a = float(sign(data.count));

	return data;
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
vec4 SceneTangent() {
	vec4 tangent = texture(sceneTex, vec3(exTexCoords, 3.0f));
	//tangent.xyz /= 2;
	//tangent.xyz += vec3(0.5f);
	return tangent;
}

layout(index = 7) subroutine(DrawTexture)
vec4 SceneBiTangent() {
	vec4 biTangent = texture(sceneTex, vec3(exTexCoords, 4.0f));
	//biTangent.xyz /= 2;
	//biTangent.xyz += vec3(0.5f);
	return biTangent;
}

layout(index = 8) subroutine(DrawTexture)
vec4 SceneDepth() {
	return texture(sceneDepth, exTexCoords);
}



vec4 voxelSampleLevel(vec3 position, float level) {
	float mip = round(level);
	float scale = float(scene.voxelRes >> int(mip));
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
	
	//vec4 total = vec4(0.0f);
	VoxelData total = {vec4(0.0f), 0, 0 };
	float count = 0.0f;
	for(int i = 0; i < 8; i++) {
		vec3 voxelPos = (positionWhole + offsets[i]) / scale;
		VoxelData voxel = unpackARGB8(textureLod(voxelData, voxelPos, mip).r);
		vec3 temp = (1.0f - offsets[i]) * (1.0f - intpol) + offsets[i] * intpol; 
		total.color += voxel.color * temp.x * temp.y * temp.z * float(sign(voxel.light));
		count += float(voxel.light) * float(sign(voxel.count)) * temp.x * temp.y * temp.z;
	}

	return total.color;//vec4(count);
}

vec4 voxelSampleBetween(vec3 position, float level) {
	float levelLow = floor(level);
	float levelHigh = levelLow + 1.0f;
	float intPol = level - levelLow;

	vec4 voxelLow = voxelSampleLevel(position, levelLow);
	vec4 voxelHigh = voxelSampleLevel(position, levelHigh);

	return voxelLow * (1 - intPol) + voxelHigh * intPol;
}

vec4 ConeTrace(vec3 startPos, vec3 dir, float coneRatio, float maxDist,	float voxelSize) {

	vec4 accum = vec4(0.0f);
	vec3 samplePos;
	vec4 sampleValue;
	float sampleRadius;
	float sampleDiameter;
	float sampleWeight;
	float sampleLOD = 0.0f;
	
	for(float dist = voxelSize; dist < maxDist && accum.a < 1.0f;) {
		sampleRadius = coneRatio * dist;
		sampleDiameter = max(2.0f * sampleRadius, voxelSize);
		sampleLOD = log2(sampleDiameter * float(scene.voxelRes));
		samplePos = startPos + dir * (dist + sampleRadius);
		sampleValue = voxelSampleBetween(samplePos, sampleLOD);
		sampleWeight = 1.0f - accum.a;
		accum += sampleValue * sampleWeight;
		dist += sampleRadius;
	}

	return accum;
}

vec4 ConeTrace60(vec3 startPos, vec3 dir, float aoDist, float maxDist, float voxelSize) {
	
	vec4 accum = vec4(0.0f);
	float opacity = 0.0f;
	vec3 samplePos;
	vec4 sampleValue;
	float sampleWeight;
	float sampleLOD = 0.0f;
	
	for(float dist = voxelSize; dist < maxDist && accum.a < 1.0f;) {
		samplePos = startPos + dir * dist;
		sampleValue = voxelSampleLevel(samplePos, sampleLOD);
		sampleWeight = 1.0f - accum.a;
		accum += sampleValue * sampleWeight;
		opacity = (dist < aoDist) ? accum.a : opacity;
		sampleLOD += 1.0f;
		dist *= 2.0f;
	}

	return vec4(accum.rgb, 1.0f - opacity);
}

vec4 DiffuseTrace () {
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
	
	float voxelSize = 1.0f / float(scene.voxelRes);
	float maxDistance = 1.0f;
	float aoDistance = 0.015f;
	vec3 pos = ScenePosition().xyz;
	vec3 norm = SceneNormal().xyz;
	vec3 tang = SceneTangent().xyz;
	vec3 bitang = SceneBiTangent().xyz;

	pos += norm * voxelSize * 2.0f;

	vec4 total = vec4(0.0f);
	for(int i = 0; i < 6; i++) {
		vec3 direction = dir[i].x * tang + dir[i].y * norm + dir[i].z * bitang;
		total += weight[i] * ConeTrace60(pos, direction, aoDistance, maxDistance, voxelSize);
	}

	return total;
}

vec4 AngleTrace(vec3 dir, float theta) {
	float voxelSize = 1.0f / float(scene.voxelRes);
	float maxDistance = sqrt(3);
	
	vec3 pos = ScenePosition().xyz;
	vec3 norm = SceneNormal().xyz;
	pos += norm * voxelSize * 2.0f;

	
	float halfTheta = sin(radians(theta)/2.0f);
	float coneRatio = halfTheta / (1 - halfTheta);
	return ConeTrace(pos, dir, coneRatio, maxDistance, voxelSize);
}

layout(index = 9) subroutine(DrawTexture)
vec4 AmbientOcclusion() {
	return vec4(vec3(DiffuseTrace().w), 1.0f);
}

layout(index = 10) subroutine(DrawTexture)
vec4 DiffuseBounce() {
	return vec4(DiffuseTrace().rgb, 1.0f);
}

layout(index = 11) subroutine(DrawTexture)
vec4 SoftShadows() {
	vec4 result = AngleTrace(scene.lightDir, 5.0f); 
	vec4 color = SceneColor();
	return vec4(color.rgb * (1.0f - result.a), 1.0f);
}

layout(index = 12) subroutine(DrawTexture)
vec4 Combination() {
	vec4 diffuse = DiffuseTrace();
	vec4 shadows = AngleTrace(scene.lightDir, 5.0f);
	vec4 color = SceneColor();
	return vec4((color.rgb * 0.9f + diffuse.rgb * 0.4f) * (1.0f - shadows.a) + diffuse.rgb * shadows.a * diffuse.a * 0.6f , 1.0f);
}

layout(location = 0) subroutine uniform DrawTexture SampleTexture;

void main()
{	
	outColor = SampleTexture();
}
