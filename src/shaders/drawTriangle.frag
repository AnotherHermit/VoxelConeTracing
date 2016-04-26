///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

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

VoxelData unpackARGB8(uint input) {
	VoxelData data;
	uvec3 uiColor;

	// Put a first to improve max operation but it should not be very noticable
	data.light = (input & 0xF0000000) >> 28;
	data.count = (input & 0x0F000000) >> 24;
	uiColor.r =  (input & 0x00FF0000) >> 16;
	uiColor.g =  (input & 0x0000FF00) >> 8;
	uiColor.b =  (input & 0x000000FF);

	data.color.rgb = vec3(uiColor) / float(data.count) / 31.0f;
	data.color.a = 1.0f;

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
	return texture(sceneTex, vec3(exTexCoords, 2.0f));
}

layout(index = 6) subroutine(DrawTexture)
vec4 SceneDepth() {
	return texture(sceneDepth, exTexCoords);
}

layout(location = 0) subroutine uniform DrawTexture SampleTexture;

void main()
{	
	outColor = SampleTexture();
}
