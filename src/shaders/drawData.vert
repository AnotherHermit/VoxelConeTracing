///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBiTangent;

out vec3 exNormal;
out vec4 exPosition;
out vec2 exTexCoords;
out vec3 exTangent;
out vec3 exBiTangent;

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

void main(void)
{
	exNormal = inNormal;
	exTangent = inTangent;
	exBiTangent = inBiTangent;
		
	vec4 temp = scene.MTOmatrix[2] * vec4(inPosition, 1.0f);
	
	temp.xyz /= 2;
	temp.xyz += vec3(0.5f);

	exPosition = temp;
	
	gl_Position = cam.VTPmatrix * cam.WTVmatrix * vec4(inPosition, 1.0f);

	exTexCoords = inTexCoords;
}
