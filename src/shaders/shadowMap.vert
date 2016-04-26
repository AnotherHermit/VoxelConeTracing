///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec3 exNormal;
//out vec4 exPosition;

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

void main(void)
{
	exNormal = mat3(scene.MTShadowMatrix * scene.MTOmatrix[2]) * inNormal;
	gl_Position = scene.MTShadowMatrix * scene.MTOmatrix[2] * vec4(inPosition, 1.0f);
}
