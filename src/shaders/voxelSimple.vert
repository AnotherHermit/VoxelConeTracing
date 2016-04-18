///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 3) in uint inVoxelPos;

out vec4 outPosition;
out vec3 outNormal;
out vec4 outColor;

layout(location = 4) uniform usampler3D voxelData;

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

uvec3 unpackRG11B10(uint input) {
	uvec3 outVec;

	outVec.r = (input & 0xFFE00000) >> 21;
	outVec.g = (input & 0x001FFC00) >> 10;
	outVec.b = (input & 0x000003FF);

	return outVec;
}

void main(void)
{
	float size = float(scene.voxelRes >> scene.mipLevel);
	vec3 voxelPos = vec3(unpackRG11B10(inVoxelPos)) / size;

	uvec4 color = unpackARGB8(textureLod(voxelData, voxelPos, float(scene.mipLevel)).r);
	outColor = vec4(color) / float(color.a);
	
	outNormal = mat3(cam.WTVmatrix) * inNormal;
	vec4 temp = cam.WTVmatrix * scene.MTWmatrix * vec4(inPosition / size + 2.0f * voxelPos - vec3(1.0f), 1.0f);
	outPosition = temp;
	gl_Position = cam.VTPmatrix * temp;
}

