///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 0) in vec3 inPosition;

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
	gl_Position = scene.MTShadowMatrix * scene.MTOmatrix[2] * vec4(inPosition, 1.0f);
}
