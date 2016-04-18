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
layout(location = 6) uniform usampler2D shadowMap;

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	mat4 MTShadowMatrix;
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

uvec4 unpackARGB8(uint input) {
	uvec4 outVec;
	
	// Put a first to improve max operation but it should not be very noticable
	outVec.a = (input & 0xFF000000) >> 24;
	outVec.r = (input & 0x00FF0000) >> 16;
	outVec.g = (input & 0x0000FF00) >> 8;
	outVec.b = (input & 0x000000FF);

	return outVec;
}

void main()
{	
	float size = float(scene.voxelRes >> scene.mipLevel);
	float depth = float(scene.voxelLayer) / size;

	vec4 color2D = vec4(unpackARGB8(texture(voxelTextures, vec3(exTexCoords, float(scene.view))).r)) / 255.0f;
	vec4 color3D = vec4(unpackARGB8(textureLod(voxelData, vec3(exTexCoords, depth), scene.mipLevel).r)) / 255.0f;
	vec4 shadowColor = vec4(unpackARGB8(texture(shadowMap, exTexCoords).r)) / 255.0f;

	outColor = color3D * float(scene.voxelDraw == 0) + color2D * float(scene.voxelDraw == 1) + shadowColor * float(scene.voxelDraw == 2);
}
